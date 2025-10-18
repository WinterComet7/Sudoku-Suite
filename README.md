# Sudoku-Suite

An app for solving and generating Sudoku puzzles, written in C with dependencies on the GTK4 library. It is mainly
focused on the algorithmic side of puzzle solving,
implementing [the Dancing Links Algorithm](https://en.wikipedia.org/wiki/Dancing_links), but
includes a simple GUI
as well.

# How to use

## Linux

1. Install dependencies **CMake, GTK4, GCC and pkg-config**
2. Clone the repository and build the project:

   ```
   git clone https://github.com/WinterComet7/Sudoku-Suite.git
   cd SudokuSuite
   mkdir build && cd build
   cmake ..
   make 
   ```

4. Run the project:

   ```
   ./GUI #Runs the program in release mode
   #or
   ./Debug #Runs the program in debug mode
   ```

## Windows (Using MSYS2)

1. Install MSYS2 from https://www.msys2.org/
2. Install dependencies **CMake, GTK4, MSVC and pkg-config**, using the **MSYS2 MinGW 64-bit shell**:

   ```
   pacman -Syu
   pacman -S --needed base-devel mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake mingw-w64-x86_64-pkg-config mingw-w64-x86_64-gtk4
   ```

3. Clone the repository and build the project:

   ```
   git clone https://github.com/WinterComet7/Sudoku-Suite.git
   cd SudokuSuite
   mkdir build && cd build
   cmake -G "MinGW Makefiles" ..
   mingw32-make
   ```

4. Run the project:

   ```
   ./GUI.exe #Runs the program in release mode
   #or
   ./Debug.exe #Runs the program in debug mode
   ```

# TO-DO list

- [ ] Update the GUI
- [ ] Add an option to generate puzzles of different sizes
- [ ] Add and update documentation
- [ ] Add more tests

# Used resources

During development, I've used the following resources, which I highly recommend checking out, should you be interested
in developing your own Sudoku solver:

- [Dancing Links Algorithm on Wikipedia](https://en.wikipedia.org/wiki/Dancing_links)
- [Glossary of Sudoku on Wikipedia](https://en.wikipedia.org/wiki/Glossary_of_Sudoku)
- [Sudoku solving algorithms on Wikipedia](https://en.wikipedia.org/wiki/Sudoku_solving_algorithms)
- [Knuth's Algorithm X on Wikipedia](https://en.wikipedia.org/wiki/Knuth%27s_Algorithm_X)
- [Exact cover problems on Wikipedia](https://en.wikipedia.org/wiki/Exact_cover#Sudoku)
- [Donald E. Knuth's paper on the Dancing Links Algorithm](https://www.ocf.berkeley.edu/~jchu/publicportal/sudoku/0011047.pdf)

The following resources were also used, serving as a guide for the implementation of the Dancing Links algorithm:

- [A full table of constraints for a regular 9x9 Sudoku puzzle](https://www.stolaf.edu/people/hansonr/sudoku/exactcovermatrix.htm)
- [Implementation of the Dancing Links Algorithm for solving Sudoku puzzles (no. 1)](https://bolota.eu/posts/14_algxsudoku)
- [Implementation of the Dancing Links Algorithm for solving Sudoku puzzles (no. 2)](https://dev.to/fahadalikhanca/implementing-the-dancing-links-dlx-algorithm-for-exact-cover-in-c-1jnp)
- [Implementation of the Dancing Links Algorithm for solving Sudoku puzzles (no. 3)](https://medium.com/@pinkudebnath/solving-sudoku-ea007ab5297b)