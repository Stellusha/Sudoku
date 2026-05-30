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
            cnt = 0;
            for (n = 1; n <= 9; n++)
                if (is_valid(g, r, c, n))
                    cnt++;
            if (cnt < best)
            {
                best = cnt;
                *out_row = r;
                *out_col = c;
                if (cnt == 0)
                    return 1;
            }
        }
    }
    return (*out_row != -1);
}

static int solve(Grid *g)
{
    int row, col, num;
    if (!find_best_empty(g, &row, &col))
        return 1;
    for (num = 1; num <= 9; num++)
    {
        if (is_valid(g, row, col, num))
        {
            g->cell[row][col] = num;
            if (solve(g))
                return 1;
            g->cell[row][col] = 0;
        }
    }
    return 0;
}

static void print_grid(const Grid *g)
{
    int r, c;
    const char *div = "+-------+-------+-------+";
    printf("%s\n", div);
    for (r = 0; r < 9; r++)
    {
        if (r == 3 || r == 6)
            printf("%s\n", div);
        printf("| ");
        for (c = 0; c < 9; c++)
        {
            if (g->cell[r][c] == 0)
                printf(".");
            else
                printf("%d", g->cell[r][c]);
            if (c == 2 || c == 5)
                printf(" | ");
            else
                printf(" ");
        }
        printf("|\n");
    }
    printf("%s\n", div);
}

static Grid parse_sdm_line(const char *line)
{
    Grid g;
    int idx = 0, i;
    memset(&g, 0, sizeof(g));
    for (i = 0; line[i] && idx < 81; i++)
    {
        char ch = line[i];
        if (isdigit(ch))
        {
            g.cell[idx / 9][idx % 9] = ch - '0';
            idx++;
        }
        else if (ch == '.')
        {
            g.cell[idx / 9][idx % 9] = 0;
            idx++;
        }
    }
    return g;
}

static Puzzle *read_sdm(const char *path, int *count)
{
    FILE *f = fopen(path, "r");
    char line[256];
    int capacity = 16;
    Puzzle *puzzles;
    *count = 0;
    if (!f)
    {
        fprintf(stderr, "Cannot open file: %s\n", path);
        return NULL;
    }
    puzzles = (Puzzle *)malloc(capacity * sizeof(Puzzle));
    while (fgets(line, sizeof(line), f))
    {
        int cnt = 0, i;
        for (i = 0; line[i]; i++)
            if (isdigit(line[i]) || line[i] == '.')
                cnt++;
        if (cnt == 81)
        {
            if (*count == capacity)
            {
                capacity *= 2;
                puzzles = (Puzzle *)realloc(puzzles, capacity * sizeof(Puzzle));
            }
            puzzles[*count].grid = parse_sdm_line(line);
            (*count)++;
        }
    }
    fclose(f);
    return puzzles;
}

static int write_sdm(const char *path, const Grid *solutions, int count)
{
    int i, r, c;
    FILE *f = fopen(path, "w");
    if (!f)
        return 0;
    for (i = 0; i < count; i++)
    {
        for (r = 0; r < 9; r++)
            for (c = 0; c < 9; c++)
                fputc('0' + solutions[i].cell[r][c], f);
        fputs("\r\n", f);
    }
    fclose(f);
    return 1;
}

static Puzzle *read_sdx(const char *path, int *count)
{
    FILE *f = fopen(path, "r");
    char line[512];
    int row = 0;
    Puzzle *p;
    *count = 0;
    if (!f)
    {
        fprintf(stderr, "Connot open file: %s\n", path);
        return NULL;
    }
    p = (Puzzle *)calloc(1, sizeof(Puzzle));
    while (row < 9 && fgets(line, sizeof(line), f))
    {
        char *ptr = line;
        int col = 0;
        while (col < 9 && *ptr)
        {
            while (*ptr == ' ' || *ptr == '\r')
                break;
            if (!*ptr || *ptr == '\n' || *ptr == '\r')
                break;
            if (!*ptr == 'u' || *ptr == 'U')
                ptr++;
            char token[16];
            int ti = 0;
            while (*ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '\n' && *ptr != '\r')
                token[ti++] = *ptr++;
            token[ti] = '\0';
            if (ti == 1 && isdigit(token[0]))
                p->grid.cell[row][col] = token[0] - '0';
            else
                p->grid.cell[row][col] = 0;
            col++;
        }
        row++;
    }
    fclose(f);
    if (row < 9)
    {
        fprintf(stderr, "SDX file has fewer than 9 rows.\n");
        free(p);
        return NULL;
    }
    *count = 1;
    return p;
}

static int write_sdx(const char *path, const Grid *g)
{
    FILE *f = fopen(path, "w");
    int r, c;
    if (!f)
        return 0;
    for (r = 0; r < 9; r++)
    {
        for (c = 0; c < 9; c++)
        {
            if (c > 0)
                fputc(' ', f);
        }
        fputs("\r\n", f);
    }
    fclose(f);
    return 1;
}

static int is_ss_divider(const char *line)
{
    int i;
    for (i = 0; line[i] && line[i] != '\n' && line[i] != '\r'; i++)
        if (line[i] != '-' && line[i] != ' ')
            return 0;
    return i > 0;
}

static Puzzle *read_ss(const char *path, int *count)
{
    FILE *f = fopen(path, "r");
    char line[128];
    int row = 0;
    Puzzle *p;
    *count = 0;
    if (!f)
    {
        fprintf(stderr, "Cannot open file: %s\n", path);
        return NULL;
    }
    p = (Puzzle *)calloc(1, sizeof(Puzzle));
    while (row < 9 && fgets(line, sizeof(line), f))
    {
        if (is_ss_divider(line))
            continue;
        char cells[16];
        int ci = 0, i;
        for (i = 0; line[i] && ci < 9; i++)
        {
            char ch = line[i];
            if (isdigit(ch) || ch == '.' || ch == 'X' || ch == 'x')
                cells[ci++] = ch;
        }
        if (ci < 9)
            continue;
        for (i = 0; i < 9; i++)
        {
            char ch = cells[i];
            p->grid.cell[row][i] = (isdigit(ch) && ch != '0') ? ch - '0' : 0;
        }
        row++;
    }
    fclose(f);
    if (row < 9)
    {
        fprintf(stderr, "SS file has fewer than 9 data rows.\n");
        free(p);
        return NULL;
    }
    *count = 1;
    return p;
}

static int write_ss(const char *path, const Grid *g)
{
    FILE *f = fopen(path, "w");
    int r, c;
    if (!f)
        return 0;
    for (r = 0; r < 9; r++)
    {
        if (r == 3 || r == 6)
            fputs("-----------\n", f);
        for (c = 0; c < 9; c++)
        {
            if (c == 3 || c == 6)
                fputc('|', f);
            int v = g->cell[r][c];
            fputc(v == 0 ? '.' : '0' + v, f);
        }
        fputc('\n', f);
    }
    fclose(f);
    return 1;
}

static const char *get_extension(const char *path)
{
    const char *dot = strrchr(path, '.');
    return dot ? dot : "";
}

static void make_output_path(const char *input, char *output, int out_size)
{
    const char *dot = strrchr(input, '.');
    if (!dot)
    {
        snprintf(output, out_size, "%s_solved", input);
    }
    else
    {
        int base_len = (int)(dot - input);
        snprintf(output, out_size, "%.s_solved%s", base_len, input, dot);
    }
}

static int ext_eq(const char *ext, const char *cmp)
{
    int i;
    for (i = 0; ext[i] && cmp[i]; i++)
        if (tolower((unsigned char)ext[i]) != tolower((unsigned char)cmp[i]))
            return 0;
    return ext[i] == '\0' && cmp[i] == '\0';
}

int main(int argc, char *argv[])
{
    char file_path[512];
    char out_path[600];
    const char *ext;
    Puzzle *puzzles = NULL;
    Grid *solutions = NULL;
    int count = 0, i;

    if (argc >= 2)
    {
        strncpy(file_path, argv[1], sizeof(file_path) - 1);
        file_path[sizeof(file_path) - 1] = '\0';
    }
    else
    {
        printf("Enter path to sudoku file (.sdx, sdm, .ss): ");
        fflush(stdout);
        if (!fgets(file_path, sizeof(file_path), stdin))
            return 1;
        file_path[strcspn(file_path, "\r\n")] = '\0';
    }

    {
        int len = (int)strlen(file_path);
        if (len >= 2 && file_path[0] == '"' && file_path[len - 1] == '"')
        {
            memmove(file_path, file_path + 1, len - 2);
            file_path[len - 2] = '\0';
        }
    }

    ext = get_extension(file_path);

    if (ext_eq(ext, ".sdm"))
        puzzles = read_sdm(file_path, &count);
    else if (ext_eq(ext, ".sdx"))
        puzzles = read_sdx(file_path, &count);
    else if (ext_eq(ext, ".ss"))
        puzzles = read_ss(file_path, &count);
    else
    {
        fprintf(stderr, "Unsupported extension: %s\n", ext);
        fprintf(stderr, "Supported: .sdm  .sdx  .ss\n");
        return 1;
    }

    if (!puzzles || count == 0)
    {
        fprintf(stderr, "No puzzles found in file.\n");
        free(puzzles);
        return 1;
    }

    printf("Found %d puzzle(s) in %s\n\n", count, file_path);

    solutions = (Grid *)malloc(count * sizeof(Grid));

    for (i = 0; i < count; i++)
    {
        printf("=== Puzzle %d ===\n", i + 1);
        printf("Input:\n");
        print_grid(&puzzles[i].grid);

        solutions[i] = puzzles[i].grid;
        if (solve(&solutions[i]))
        {
            printf("Solution:\n");
            print_grid(&solutions[i]);
        }
        else
        {
            printf("No solution found for puzzle %d.\n", i + 1);
            solutions[i] = puzzles[i].grid; /* keep original */
        }
    }

    make_output_path(file_path, out_path, sizeof(out_path));

    {
        int written = 0;
        if (ext_eq(ext, ".sdm"))
            written = write_sdm(out_path, solutions, count);
        else if (ext_eq(ext, ".sdx"))
            written = write_sdx(out_path, &solutions[0]);
        else if (ext_eq(ext, ".ss"))
            written = write_ss(out_path, &solutions[0]);

        if (written)
            printf("\nSolutions saved to: %s\n", out_path);
        else
            fprintf(stderr, "\nFailed to write output file.\n");
    }

    free(puzzles);
    free(solutions);
    return 0;
}
