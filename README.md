# 🦀 Pagurus

A tiny terminal game written in C++ with ncurses: walk a crab around a grassy field and collect coins.

```
Coins: 3
+----------------------------------------------------------------+
|`  .``    '  .         `         ,   ` '    ,   ,    '  `  '. ".|
|            ,'    '     "   `. ``     ' .    '` .       "    '` |
|  "      ,     "` `    ,  ' ."  ,.  ",  .   , ,'.        ,    , |
|     .         " ''    $    ,  .   "   `     ,     '' ". ,"  .` |
|    "    ,             ' (\/)(o,,o)(\/)         '        "'  ` `|
|      "  ''              `'. `/  \   " ,    `  '   . ,         "|
|   .   "         " `  `         `       ,'      , ," .    ",    |
+----------------------------------------------------------------+
```

## Features

- A crab that moves with the arrow keys and blinks every few seconds
- A fenced 64×32 field with randomly generated grass
- Collect randomly spawned coins

## Requirements

- A C++17 compiler (GCC or Clang)
- CMake 3.16+
- ncurses

| Distro | Install |
|---|---|
| Arch / CachyOS | `sudo pacman -S base-devel cmake ncurses` |
| Debian / Ubuntu | `sudo apt install build-essential cmake libncurses-dev` |
| Fedora | `sudo dnf install gcc-c++ cmake ncurses-devel` |
| macOS | `xcode-select --install && brew install cmake` |

## Build and run


| Command | Discription |
|---|---|
| `make` | Build the game into `build/` |
| `make run` | Build if needed, then start the game |
| `make clean` | Delete the `build/` folder |
| `make BUILD_TYPE=Debug` | Debug build (run `make clean` first if `build/` exists) |

Without make: `cmake -S . -B build && cmake --build build && ./build/pagurus`.

## Controls

| Key | Action |
|---|---|
| ← ↑ → ↓ | Move the crab |
| `q` | Quit |

For the full field, use a terminal of at least 66×35 characters.

## Customizing

All tunable values live in the **Settings** section at the top of [`src/main.cpp`](src/main.cpp): the crab drawing (`kBody`, `kLegs`), blink timing, the coin character, field size, and grass characters and density.

## Project structure

```
Pagurus/
├── src/main.cpp     # the whole game
├── CMakeLists.txt   # build configuration
└── Makefile         # shortcuts: make, make run, make clean
```

## License

[MIT](LICENSE) © 2026 smvfe
