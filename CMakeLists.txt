cmake_minimum_required(VERSION 3.31)
project(SudokuSuite C)

set(CMAKE_C_STANDARD 23)

# Find GTK 4 using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)

# Source files:
# Format: add_executable(<config_name> <file.c>)
add_executable(Algorithm algorithm.c)
add_executable(GUI gui.c)
add_executable(Main main.c)

# GTK library dependencies:
# Format: target_link_libraries(<config_name> PRIVATE PkgConfig::GTK4)
target_link_libraries(GUI PRIVATE PkgConfig::GTK4)
