#ifndef ALGORITHM_H
#define ALGORITHM_H

typedef enum DIFFICULTY DIFFICULTY;

enum DIFFICULTY
{
    EASY = 12,
    MEDIUM = 6,
    HARD = 3,
    EXTREME = 0,
};

int* dlx_sudoku_generate_unique_unsolved(int sudoku_size, enum DIFFICULTY additional_constraints);
int* dlx_sudoku_solve(int* sudoku_grid_unsolved, int sudoku_size);

#endif
