#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "algorithm.h"

typedef struct SudokuGrid
{
    int grid_size; // Size of the Sudoku grid, measured in the number of horizontal cells.
    int box_size; // Size of one box in the Sudoku grid, measured in the number of horizontal cells.

    int digits_len; // Length of the <digits> array. Is equal to 'pow(grid_size, 2)'.
    int* digits_unsolved; // Digits of the initial grid.
    int* digits_solved; // Digits of the solved grid.
} SudokuGrid;

// Utility functions (Printing):
void sudoku_print_border_horizontal(int sudoku_size)
{
    int box_size = (int)sqrt(sudoku_size);
    for (int ix = 0; ix < sudoku_size; ix++)
    {
        bool add_border_vertical = ((ix + 1) % box_size == 0);
        bool end_row = ((ix + 1) % sudoku_size == 0);
        printf("-%s", (end_row) ? "" : "-");

        if (add_border_vertical && !end_row)
            printf("+-");
        if (end_row)
            printf("\n");
    }
}

void sudoku_print(int* sudoku_grid, int sudoku_size)
{
    int box_size = (int)sqrt(sudoku_size);
    int digits_len = (int)pow(sudoku_size, 2);
    int filled_cells = 0;

    for (int ix = 0; ix < digits_len; ix++)
    {
        bool add_border_vertical = ((ix + 1) % box_size == 0);
        bool add_border_horizontal = ((ix + 1) % (sudoku_size * box_size) == 0);
        bool end_row = ((ix + 1) % sudoku_size == 0);
        bool last_row = (ix == digits_len - 1);

        printf("%d%s", sudoku_grid[ix], (end_row) ? "" : " ");
        if (sudoku_grid[ix] > 0) filled_cells++;

        if (add_border_vertical && !end_row)
            printf("| ");
        if (end_row)
            printf("\n");
        if (add_border_horizontal && !last_row)
            sudoku_print_border_horizontal(sudoku_size);
    }
    printf("[Filled cells: %d]\n\n", filled_cells);
}

// Functions for grid creation/deletion:
SudokuGrid* sudoku_initialize_empty(int sudoku_size)
{
    int digits_len = (int)pow(sudoku_size, 2);

    SudokuGrid* sudoku = (SudokuGrid*)malloc(sizeof(SudokuGrid) + digits_len * sizeof(int));
    sudoku->grid_size = sudoku_size;
    sudoku->box_size = (int)sqrt(sudoku_size);

    sudoku->digits_len = digits_len;
    sudoku->digits_unsolved = (int*)calloc(digits_len, sizeof(int));
    sudoku->digits_solved = (int*)calloc(digits_len, sizeof(int));

    return sudoku;
}

SudokuGrid* sudoku_initialize_complete(int sudoku_size, enum DIFFICULTY additional_constraints)
{
    int digits_len = (int)pow(sudoku_size, 2);

    SudokuGrid* sudoku = (SudokuGrid*)malloc(sizeof(SudokuGrid) + digits_len * sizeof(int));
    sudoku->grid_size = sudoku_size;
    sudoku->box_size = (int)sqrt(sudoku_size);

    sudoku->digits_len = digits_len;
    sudoku->digits_unsolved = dlx_sudoku_generate_unique_unsolved(sudoku_size, additional_constraints);
    sudoku->digits_solved = dlx_sudoku_solve(sudoku->digits_unsolved, sudoku_size);

    return sudoku;
}

SudokuGrid* sudoku_copy(SudokuGrid* sudoku_grid)
{
    SudokuGrid* copy = sudoku_initialize_empty(sudoku_grid->grid_size);

    for (int ix = 0; ix < sudoku_grid->digits_len; ix++)
    {
        copy->digits_unsolved[ix] = sudoku_grid->digits_unsolved[ix];
        copy->digits_solved[ix] = sudoku_grid->digits_solved[ix];
    }

    return copy;
}

void sudoku_terminate(SudokuGrid* sudoku_grid)
{
    free(sudoku_grid->digits_unsolved);
    free(sudoku_grid->digits_solved);
    free(sudoku_grid);
}

void sudoku_set_constraint(SudokuGrid* sudoku_grid, int sudoku_row, int sudoku_column, int sudoku_value)
{
    int sudoku_size = sudoku_grid->grid_size;
    if (sudoku_row < 1 || sudoku_row > sudoku_size ||
        sudoku_column < 1 || sudoku_column > sudoku_size ||
        sudoku_value < 1 || sudoku_value > sudoku_size)
    {
        fprintf(stderr, "ERROR: Invalid coordinates or value!\n");
        exit(EXIT_FAILURE);
    }

    int grid_index = (sudoku_row - 1) * sudoku_size + sudoku_column - 1;
    sudoku_grid->digits_unsolved[grid_index] = sudoku_value;
}

int main()
{
    // Change the parameters of the executed test here:
    int sudoku_size = 9;
    DIFFICULTY difficulty = EXTREME;

    // Changing the code below this comment will alter the execution of the algorithm. It is advised to have a brief understanding of the functions used before proceeding.
    SudokuGrid* sudoku = sudoku_initialize_complete(sudoku_size, difficulty);
    sudoku_print(sudoku->digits_unsolved, sudoku_size);
    sudoku_print(sudoku->digits_solved, sudoku_size);

    sudoku_terminate(sudoku);
    return 0;
}
