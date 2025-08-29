cmake_minimum_required(VERSION 3.31)
project(SudokuSuite C)

set(CMAKE_C_STANDARD 23)

# Find GTK 4 using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)

# Source files:
# Format: add_executable(<executable_name> <name.c> <name.h>)
add_executable(GUI
        gui.c)

add_executable(Main
        sudoku_main.c
        sudoku_algorithm.c
        sudoku_main.h
        sudoku_algorithm.h)
target_compile_definitions(Main PRIVATE MAIN_ACTIVE=true)

# GTK library dependencies:
# Format: target_link_libraries(<config_name> PRIVATE PkgConfig::GTK4)
target_link_libraries(GUI PRIVATE PkgConfig::GTK4)
