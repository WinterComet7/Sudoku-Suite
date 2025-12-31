#ifndef SUDOKUSUITE_ALGORITHM_H
#define SUDOKUSUITE_ALGORITHM_H
#include "sudoku_main.h"

enum DIFFICULTY
{
    EASY    = 18,
    MEDIUM  = 12,
    HARD    = 6,
    EXTREME = 0,
};

SudokuGrid* dlx_sudoku_generate_unique_pair(int sudoku_size, enum DIFFICULTY additional_constraints);
int*        dlx_sudoku_solve(int* sudoku_grid_unsolved, int sudoku_size);

#endif // SUDOKUSUITE_ALGORITHM_H
