#ifndef SUDOKUSUITE_SUDOKU_H
#define SUDOKUSUITE_SUDOKU_H

typedef struct SudokuGrid
{
    int sudoku_cells; // Length of the <digits> array. Is equal to 'pow(grid_size, 2)'.
    int sudoku_size; // Size of the Sudoku grid, measured in the number of horizontal cells.
    int sudoku_block_size; // Size of one box in the Sudoku grid, measured in the number of horizontal cells.

    int* sudoku_unsolved; // Digits in the cells of the grid.
    int* sudoku_solved; // Digits in the cells of the grid, after solving.
} SudokuGrid;

SudokuGrid* grid_create(int grid_size);

#endif //SUDOKUSUITE_SUDOKU_H
