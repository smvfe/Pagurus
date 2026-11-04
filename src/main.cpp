#include <ncurses.h>

#include <algorithm>
#include <chrono>
#include <random>
#include <string>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;

// ---- Settings ---------------------------------------------------------------

const std::string kBody       = "(\\/)(o,,o)(\\/)";
const std::string kLegs       = "     /  \\     ";
const int kCrabHeight = 2;
const int kCrabWidth  = static_cast<int>(std::max(kBody.size(), kLegs.size()));
const char kEyeOpen   = 'o';
const char kEyeClosed = '-';

// The eyes close for kBlinkDuration at the end of every kBlinkInterval.
constexpr auto kBlinkInterval = std::chrono::milliseconds(3000);
constexpr auto kBlinkDuration = std::chrono::milliseconds(150);

const char kCoin = '$';
const int kHudHeight = 1;  // Top row is reserved for the score.

const int kFieldWidth  = 64;
const int kFieldHeight = 32;
const std::string kGrass = ",`'\".";
const int kGrassPercent = 10;  // Share of field cells that get a grass blade.

constexpr int kFrameMs = 50;  // How long getch() waits for a key before the next frame.

// ---- Game state -------------------------------------------------------------

struct Point {
    int y;
    int x;
};

// Where the field lies along one screen axis. The fence sits just outside it.
struct Span {
    int start;   // First screen cell of the field.
    int length;  // Number of cells the field covers.
};

struct Field {
    Span y;
    Span x;
    std::vector<std::string> grass;  // kFieldHeight rows of kFieldWidth cells.
};

// Crab and coin positions are relative to the field's top-left cell.
struct Game {
    Field field;
    Point crab;
    Point coin;
    int coins = 0;
    std::mt19937 rng;
    Clock::time_point start;
};

// ---- Terminal ---------------------------------------------------------------

void initTerminal() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    timeout(kFrameMs);  // getch() returns ERR after this long, so the blink keeps animating.
}

// ---- Field ------------------------------------------------------------------

std::vector<std::string> makeGrass(std::mt19937& rng) {
    std::uniform_int_distribution<int> percent(0, 99);
    std::uniform_int_distribution<std::size_t> blade(0, kGrass.size() - 1);
    std::vector<std::string> grass(kFieldHeight, std::string(kFieldWidth, ' '));
    for (std::string& row : grass) {
        for (char& cell : row) {
            if (percent(rng) < kGrassPercent) {
                cell = kGrass[blade(rng)];
            }
        }
    }
    return grass;
}

// Centers the field and its fence in the available cells, shrinking the
// field when the field plus a fence on each end does not fit.
Span fitSpan(int offset, int available, int size) {
    const int length = std::max(0, std::min(size, available - 2));
    return {offset + 1 + (available - 2 - length) / 2, length};
}

// Fits the field into the current terminal size, below the score row.
void layoutField(Field& field) {
    int rows = 0;
    int cols = 0;
    getmaxyx(stdscr, rows, cols);
    field.y = fitSpan(kHudHeight, rows - kHudHeight, kFieldHeight);
    field.x = fitSpan(0, cols, kFieldWidth);
}

bool inField(const Field& field, const Point& cell) {
    return cell.y >= 0 && cell.y < field.y.length && cell.x >= 0 && cell.x < field.x.length;
}

Point toScreen(const Field& field, const Point& cell) {
    return {field.y.start + cell.y, field.x.start + cell.x};
}

// ---- Crab -------------------------------------------------------------------

void moveCrab(Point& crab, int key) {
    switch (key) {
        case KEY_UP:    crab.y -= 2; break;
        case KEY_DOWN:  crab.y += 2; break;
        case KEY_LEFT:  crab.x -= 2; break;
        case KEY_RIGHT: crab.x += 2; break;
    }
}

void keepCrabInField(Point& crab, const Field& field) {
    crab.y = std::max(0, std::min(crab.y, field.y.length - kCrabHeight));
    crab.x = std::max(0, std::min(crab.x, field.x.length - kCrabWidth));
}

// The crab's hitbox is its whole bounding box.
bool crabCovers(const Point& crab, const Point& cell) {
    return cell.y >= crab.y && cell.y < crab.y + kCrabHeight &&
           cell.x >= crab.x && cell.x < crab.x + kCrabWidth;
}

bool eyesClosed(Clock::time_point start) {
    const auto elapsed = Clock::now() - start;
    return elapsed % kBlinkInterval >= kBlinkInterval - kBlinkDuration;
}

// ---- Coin -------------------------------------------------------------------

void placeCoin(Game& game) {
    std::uniform_int_distribution<int> randY(0, std::max(0, game.field.y.length - 1));
    std::uniform_int_distribution<int> randX(0, std::max(0, game.field.x.length - 1));
    // Avoid spawning under the crab; give up on fields too small for that.
    for (int attempt = 0; attempt < 100; ++attempt) {
        game.coin = {randY(game.rng), randX(game.rng)};
        if (!crabCovers(game.crab, game.coin)) {
            return;
        }
    }
}

void updateCoin(Game& game) {
    if (!inField(game.field, game.coin)) {  // The terminal shrank.
        placeCoin(game);
    }
    if (crabCovers(game.crab, game.coin)) {
        ++game.coins;
        placeCoin(game);
    }
}

// ---- Drawing ----------------------------------------------------------------

// Draws one line of a sprite, letting the background show through the
// spaces before and after it.
void drawSprite(int y, int x, const std::string& line) {
    const std::size_t first = line.find_first_not_of(' ');
    if (first == std::string::npos) {
        return;
    }
    const std::size_t last = line.find_last_not_of(' ');
    mvaddnstr(y, x + static_cast<int>(first), line.c_str() + first, static_cast<int>(last - first + 1));
}

void drawScore(const Game& game) {
    mvprintw(game.field.y.start - 2, game.field.x.start - 1, "Coins: %d", game.coins);
}

void drawGrass(const Field& field) {
    for (int row = 0; row < field.y.length; ++row) {
        mvaddnstr(field.y.start + row, field.x.start, field.grass[row].c_str(), field.x.length);
    }
}

void drawFence(const Field& field) {
    const int top    = field.y.start - 1;
    const int bottom = field.y.start + field.y.length;
    const int left   = field.x.start - 1;
    const int right  = field.x.start + field.x.length;
    mvhline(top, field.x.start, '-', field.x.length);
    mvhline(bottom, field.x.start, '-', field.x.length);
    mvvline(field.y.start, left, '|', field.y.length);
    mvvline(field.y.start, right, '|', field.y.length);
    mvaddch(top, left, '+');
    mvaddch(top, right, '+');
    mvaddch(bottom, left, '+');
    mvaddch(bottom, right, '+');
}

void drawCoin(const Field& field, const Point& coin) {
    const Point at = toScreen(field, coin);
    mvaddch(at.y, at.x, kCoin);
}

void drawCrab(const Field& field, const Point& crab, bool closedEyes) {
    std::string body = kBody;
    if (closedEyes) {
        std::replace(body.begin(), body.end(), kEyeOpen, kEyeClosed);
    }
    const Point at = toScreen(field, crab);
    drawSprite(at.y, at.x, body);
    drawSprite(at.y + 1, at.x, kLegs);
}

void draw(const Game& game) {
    erase();
    drawScore(game);
    drawGrass(game.field);
    drawFence(game.field);
    drawCoin(game.field, game.coin);
    drawCrab(game.field, game.crab, eyesClosed(game.start));
    refresh();
}

// ---- Game loop --------------------------------------------------------------

Game newGame() {
    Game game;
    game.rng.seed(std::random_device{}());
    game.start = Clock::now();
    game.field.grass = makeGrass(game.rng);
    layoutField(game.field);
    game.crab = {(game.field.y.length - kCrabHeight) / 2, (game.field.x.length - kCrabWidth) / 2};
    keepCrabInField(game.crab, game.field);
    placeCoin(game);
    return game;
}

void update(Game& game, int key) {
    moveCrab(game.crab, key);
    layoutField(game.field);  // Also picks up terminal resizes.
    keepCrabInField(game.crab, game.field);
    updateCoin(game);
}

}  // namespace

int main() {
    initTerminal();
    Game game = newGame();
    for (int key = 0; key != 'q'; key = getch()) {
        update(game, key);
        draw(game);
    }
    endwin();
    return 0;
}
