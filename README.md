# Sudoku Suite

Sudoku Suite is a C application for solving Sudoku puzzles and generating 9x9 Sudoku puzzles with unique solutions. The project focuses mainly on the algorithmic implementation, especially the Dancing Links algorithm, and also includes a simple GTK 4 GUI.

## Features

- Generate Sudoku puzzles with different difficulty levels
- Solve existing Sudoku boards
- Play through a simple GTK 4 graphical interface
- Run a terminal/debug version for algorithm testing

## Requirements

- CMake 3.31 or newer
- A C23-compatible C compiler
- GTK 4
- pkg-config

## How to build and run

### Linux

The following commands are intended for Debian/Ubuntu-based distributions.

1. Install the required dependencies:

   ```bash
   sudo apt install build-essential cmake pkg-config libgtk-4-dev
   ```

2. Clone the repository and build the project:

   ```bash
   git clone https://github.com/winter-comet/Sudoku-Suite.git
   cd Sudoku-Suite
   mkdir build && cd build
   cmake ..
   make
   ```

3. Run the application:

   ```bash
   ./GUI
   ```

   Or run the debug/terminal version:

   ```bash
   ./Debug
   ```

### Windows using MSYS2

1. Install MSYS2 from <https://www.msys2.org/>.

2. Open the MSYS2 MinGW 64-bit shell and install the required dependencies:

   ```bash
   pacman -Syu
   pacman -S --needed base-devel git mingw-w64-x86_64-toolchain \
     mingw-w64-x86_64-cmake mingw-w64-x86_64-pkg-config \
     mingw-w64-x86_64-gtk4
   ```

3. Clone the repository and build the project:

   ```bash
   git clone https://github.com/winter-comet/Sudoku-Suite.git
   cd Sudoku-Suite
   mkdir build && cd build
   cmake -G "MinGW Makefiles" ..
   mingw32-make
   ```

4. Run the application:

   ```bash
   ./GUI.exe
   ```

   Or run the debug/terminal version:

   ```bash
   ./Debug.exe
   ```

## To-do

- [ ] Update the GUI
- [ ] Add support for generating puzzles of different sizes
- [ ] Add and update documentation
- [ ] Add more tests

## Resources used

During development, I used the following resources, which I recommend for anyone interested in developing a Sudoku solver:

- [Dancing Links algorithm on Wikipedia](https://en.wikipedia.org/wiki/Dancing_links)
- [Glossary of Sudoku on Wikipedia](https://en.wikipedia.org/wiki/Glossary_of_Sudoku)
- [Sudoku solving algorithms on Wikipedia](https://en.wikipedia.org/wiki/Sudoku_solving_algorithms)
- [Knuth's Algorithm X on Wikipedia](https://en.wikipedia.org/wiki/Knuth%27s_Algorithm_X)
- [Exact cover problems on Wikipedia](https://en.wikipedia.org/wiki/Exact_cover#Sudoku)
- [Donald E. Knuth's paper on the Dancing Links algorithm](https://www.ocf.berkeley.edu/~jchu/publicportal/sudoku/0011047.pdf)

The following resources were also used as guides for implementing the Dancing Links algorithm:

- [A full table of constraints for a regular 9x9 Sudoku puzzle](https://www.stolaf.edu/people/hansonr/sudoku/exactcovermatrix.htm)
- [Implementation of the Dancing Links algorithm for solving Sudoku puzzles (no. 1)](https://bolota.eu/posts/14_algxsudoku)
- [Implementation of the Dancing Links algorithm for solving Sudoku puzzles (no. 2)](https://dev.to/fahadalikhanca/implementing-the-dancing-links-dlx-algorithm-for-exact-cover-in-c-1jnp)
- [Implementation of the Dancing Links algorithm for solving Sudoku puzzles (no. 3)](https://medium.com/@pinkudebnath/solving-sudoku-ea007ab5297b)
