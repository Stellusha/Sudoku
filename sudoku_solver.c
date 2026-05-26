#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct
{
    int cell[9][9];
} Grid;

typedef struct
{
    Grid grid;
} Puzzle;

static int is_valid(const Grid *g, int row, int col, int num)
{
    int r, c, br, bc;
    for (c = 0; c < 9; c++)
        if (g->cell[row][c] == num)
            return 0;
    for (r = 0; r < 9; r++)
        if (g->cell[r][col] == num)
            return 0;
    br = (row / 3) * 3;
    bc = (col / 3) * 3;
    for (r = 0; r < 3; r++)
        for (c = 0; c < 3; c++)
            if (g->cell[br + r][bc + c] == num)
                return 0;
    return 1;
}

static int find_best_empty(const Grid *g, int *out_row, int *out_col)
{
    int r, c, n, cnt, best = 10;
    *out_row = -1;
    *out_col = -1;
    for (r = 0; r < 9; r++)
    {
        for (c = 0; c < 9; c++)
        {
            if (g->cell[r][c] != 0)
                continue;
        }
    }
}