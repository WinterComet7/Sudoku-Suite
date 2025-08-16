#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define GRID_SIZE 9

/*

    +-----------------------------------------+
    | Note: Works only with 9x9 Sudoku grids! |
    +-----------------------------------------+

    +-------------------------------------------------------------------------------+
    | Note: Upper limit: 36x36 Sudoku grid (max capacity of unsigned int in Matrix) |
    +-------------------------------------------------------------------------------+

*/

typedef struct SudokuGrid
{
    int grid_size; // Size of the Sudoku grid, measured in the number of horizontal cells.
    int box_size; // Size of one box in the Sudoku grid, measured in the number of horizontal cells.

    int digits_len; // Length of the <digits> array. Is equal to 'pow(grid_size, 2)'.
    int digits[]; // Digits in the cells of the grid.
} SudokuGrid;

// Utility functions (RNG):
void random_initialize()
{
    srand(time(NULL));
}

int random_interval(int min_value, int max_value)
{
    return ((rand() % (max_value - min_value + 1)) + min_value);
}

// Utility functions (Printing):
void grid_print_border_horizontal(SudokuGrid* grid)
{
    for (int ix = 0; ix < grid->grid_size; ix++)
    {
        bool add_border_vertical = ((ix + 1) % grid->box_size == 0);
        bool end_row = ((ix + 1) % grid->grid_size == 0);
        printf("-%s", (end_row) ? "" : "-");

        if (add_border_vertical && !end_row)
            printf("+-");
        if (end_row)
            printf("\n");
    }
}

void grid_print(SudokuGrid* grid)
{
    for (int ix = 0; ix < grid->digits_len; ix++)
    {
        bool add_border_vertical = ((ix + 1) % grid->box_size == 0);
        bool add_border_horizontal = ((ix + 1) % (grid->grid_size * grid->box_size) == 0);
        bool end_row = ((ix + 1) % grid->grid_size == 0);
        bool last_row = end_row && (ix == grid->digits_len - 1);

        printf("%d%s", grid->digits[ix], (end_row) ? "" : " ");

        if (add_border_vertical && !end_row)
            printf("| ");
        if (end_row)
            printf("\n");
        if (add_border_horizontal && !last_row)
            grid_print_border_horizontal(grid);
    }
}

// Functions for grid creation/deletion:
SudokuGrid* grid_create(int grid_size)
{
    int digits_len = (int)pow(grid_size, 2);

    SudokuGrid* sudoku = (SudokuGrid*)malloc(sizeof(SudokuGrid) + digits_len * sizeof(int));
    sudoku->grid_size = grid_size;
    sudoku->box_size = (int)sqrt(grid_size);

    sudoku->digits_len = digits_len;
    memset(sudoku->digits, 0, digits_len * sizeof(int));

    return sudoku;
}

SudokuGrid* grid_copy(SudokuGrid* grid)
{
    SudokuGrid* copy = grid_create(grid->grid_size);

    for (int ix = 0; ix < grid->digits_len; ix++)
        copy->digits[ix] = grid->digits[ix];

    return copy;
}

void grid_delete(SudokuGrid* grid)
{
    free(grid);
}

// Functions for setting a value in a grid (Unchecked):
void grid_set_value(SudokuGrid* grid, int row_ix, int col_ix, int value)
{
    if ((row_ix >= grid->grid_size) || (row_ix < 0) ||
        (col_ix >= grid->grid_size) || (col_ix < 0) ||
        (value > grid->grid_size) || (value < 0))
    {
        fprintf(stderr, "ERROR: Invalid coordinates or value!\n");
        exit(EXIT_FAILURE);
    }

    int index = row_ix * grid->grid_size + col_ix;
    grid->digits[index] = value;
}

int main()
{
    random_initialize();

    int grid_size = GRID_SIZE;
    SudokuGrid* test = grid_create(grid_size);
    grid_set_value(test, 0, 0, 1);
    grid_set_value(test, 0, 1, 2);
    grid_set_value(test, 0, 2, 3);
    grid_set_value(test, 0, 2, 4);

    grid_print(test);

    grid_delete(test);
    return 0;
}
