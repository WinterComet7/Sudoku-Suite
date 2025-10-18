#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sudoku_algorithm.h"
#include "sudoku_main.h"

typedef struct SudokuGrid
{
    int sudoku_cells; // Length of the <digits> array. Is equal to 'pow(grid_size, 2)'.
    int sudoku_size; // Size of the Sudoku grid, measured in the number of horizontal cells.
    int sudoku_block_size; // Size of one box in the Sudoku grid, measured in the number of horizontal cells.

    int* sudoku_unsolved; // Digits in the cells of the grid.
    int* sudoku_solved; // Digits in the cells of the grid, after solving.
} SudokuGrid;

// Utility functions (Printing):
void grid_print_border_horizontal(SudokuGrid* grid)
{
    for (int ix = 0; ix < grid->sudoku_size; ix++)
    {
        bool add_border_vertical = ((ix + 1) % grid->sudoku_block_size == 0);
        bool end_row = ((ix + 1) % grid->sudoku_size == 0);
        printf("-%s", (end_row) ? "" : "-");

        if (add_border_vertical && !end_row)
            printf("+-");
        if (end_row)
            printf("\n");
    }
}

void grid_print(SudokuGrid* grid, bool solved)
{
    int* chosen_board = (solved) ? grid->sudoku_solved : grid->sudoku_unsolved;
    int filled_cells = 0;

    for (int ix = 0; ix < grid->sudoku_cells; ix++)
    {
        bool add_border_vertical = ((ix + 1) % grid->sudoku_block_size == 0);
        bool add_border_horizontal = ((ix + 1) % (grid->sudoku_size * grid->sudoku_block_size) == 0);
        bool end_row = ((ix + 1) % grid->sudoku_size == 0);
        bool last_row = end_row && (ix == grid->sudoku_cells - 1);

        int cell_value = chosen_board[ix];
        if (cell_value > 0) filled_cells++;
        printf("%d%s", cell_value, (end_row) ? "" : " ");

        if (add_border_vertical && !end_row)
            printf("| ");
        if (end_row)
            printf("\n");
        if (add_border_horizontal && !last_row)
            grid_print_border_horizontal(grid);
    }

    printf("[Filled cells: %d]", filled_cells);
    printf("\n");
}

// Functions for grid creation/deletion:
SudokuGrid* grid_create(int grid_size)
{
    int sudoku_cells = (int)pow(grid_size, 2);

    SudokuGrid* sudoku = (SudokuGrid*)malloc(sizeof(SudokuGrid) + sudoku_cells * sizeof(int));
    sudoku->sudoku_block_size = (int)sqrt(grid_size);
    sudoku->sudoku_size = grid_size;
    sudoku->sudoku_cells = sudoku_cells;

    sudoku->sudoku_unsolved = (int*)calloc(sudoku_cells, sizeof(int));
    sudoku->sudoku_solved = (int*)calloc(sudoku_cells, sizeof(int));

    memset(sudoku->sudoku_unsolved, 0, sudoku_cells * sizeof(int));
    memset(sudoku->sudoku_solved, 0, sudoku_cells * sizeof(int));

    return sudoku;
}

SudokuGrid* grid_copy(SudokuGrid* grid)
{
    SudokuGrid* copy = grid_create(grid->sudoku_size);

    for (int ix = 0; ix < grid->sudoku_cells; ix++)
    {
        copy->sudoku_unsolved[ix] = grid->sudoku_unsolved[ix];
        copy->sudoku_solved[ix] = grid->sudoku_solved[ix];
    }

    return copy;
}

void grid_delete(SudokuGrid* grid)
{
    free(grid->sudoku_unsolved);
    free(grid->sudoku_solved);
    free(grid);
}

// Functions for setting a value in a grid (Unchecked):
void grid_set_value(SudokuGrid* grid, int sudoku_row, int sudoku_column, int sudoku_value)
{
    if (sudoku_row < 1 || sudoku_row > grid->sudoku_size ||
        sudoku_column < 1 || sudoku_column > grid->sudoku_size ||
        sudoku_value < 1 || sudoku_value > grid->sudoku_size)
    {
        fprintf(stderr, "[ERR msg] Constraint r%dc%d#%d not set! (value out of bounds)\n", sudoku_row,
                sudoku_column,
                sudoku_value);
        return;
    }
    int grid_index = (sudoku_row - 1) * grid->sudoku_size + sudoku_column - 1;
    grid->sudoku_unsolved[grid_index] = sudoku_value;
}

#ifndef GUI_ACTIVE
int main()
{
    // Do not change this line to allow random puzzle generation.
    srand((unsigned int)time(NULL));

    // Change the parameters of the executed test here:
    int sudoku_size = 9;
    enum DIFFICULTY sudoku_difficulty = EXTREME;

    // The execution of the algorithm in the terminal:
    SudokuGrid* grid = dlx_sudoku_generate_unique_pair(sudoku_size, sudoku_difficulty);

    grid_print(grid, false);
    printf("\n");

    grid_print(grid, true);
    printf("\n");

    grid_delete(grid);
    return 0;
}
#endif
