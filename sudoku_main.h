#ifndef SUDOKUSUITE_SUDOKU_H
#define SUDOKUSUITE_SUDOKU_H

#include <stdbool.h>

typedef struct SudokuGrid
{
    int sudoku_cells;      // Length of the <digits> array. Is equal to 'pow(grid_size, 2)'.
    int sudoku_size;       // Size of the Sudoku grid, measured in the number of horizontal cells.
    int sudoku_block_size; // Size of one box in the Sudoku grid, measured in the number of horizontal cells.

    int* sudoku_unsolved; // Digits in the cells of the grid.
    int* sudoku_solved;   // Digits in the cells of the grid, after solving.
} SudokuGrid;

SudokuGrid* grid_create(int grid_size);
SudokuGrid* grid_copy(SudokuGrid* grid);
void        grid_delete(SudokuGrid* grid);
void        grid_print(SudokuGrid* grid, bool solved);
void        grid_set_value(SudokuGrid* grid, int sudoku_row, int sudoku_column, int sudoku_value);
int         grid_count_filled_cells(int* sudoku_grid, int sudoku_size);

#endif // SUDOKUSUITE_SUDOKU_H
