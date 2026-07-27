cmake_minimum_required(VERSION 3.31)
project(SudokuSuite C)

set(CMAKE_C_STANDARD 23)

# Find GTK 4 using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4>=4.10)

# Source files:
# Format: add_executable(<executable_name> <name.c> <name.h>)
add_executable(GUI WIN32
        gui.c
        sudoku_main.c
        sudoku_algorithm.c
        sudoku_main.h
        sudoku_algorithm.h)
target_compile_definitions(GUI PRIVATE GUI_ACTIVE=true)

add_executable(Debug
        sudoku_main.c
        sudoku_algorithm.c
        sudoku_main.h
        sudoku_algorithm.h)
target_compile_definitions(Debug PRIVATE DEBUG_ACTIVE=true)

# Linking against the imported target (instead of GTK4_LIBRARIES) so the linker
# also gets GTK4's library search path, not just the bare library names.
target_link_libraries(GUI PRIVATE PkgConfig::GTK4)

# Copy style.css into the build directory so the binary can load it by relative path
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gui.css ${CMAKE_CURRENT_BINARY_DIR}/gui.css COPYONLY)
