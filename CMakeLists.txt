cmake_minimum_required(VERSION 3.16)
project(Pagurus LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)

add_executable(pagurus src/main.cpp)
target_include_directories(pagurus PRIVATE ${CURSES_INCLUDE_DIRS})
target_link_libraries(pagurus PRIVATE ${CURSES_LIBRARIES})
target_compile_options(pagurus PRIVATE -Wall -Wextra -Wpedantic)
