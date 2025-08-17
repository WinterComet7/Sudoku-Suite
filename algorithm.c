#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// Additional info:
// (!) - This group of methods only works on completely uncovered matrices.

typedef struct Node Node;
typedef struct HeaderNode HeaderNode;

// [TEMP] Temporary code, used exclusively for testing. This code will be removed later.
static int test = 0;

struct Node
{
    int sudoku_row, sudoku_column, sudoku_value;
    Node *left, *right, *up, *down;
    HeaderNode* column_node;
};

struct HeaderNode
{
    Node node;
    int total_size; // Total number of children nodes, both covered and uncovered.
    int current_size; // Number of children nodes in the column that are currently uncovered.
    int num;
};

// Navigation functions (!):
Node* dlx_get_node(HeaderNode* header_ptr, int node_row, int node_column)
{
    if (node_row < 0 || node_column < 0 || node_column > header_ptr->total_size)
    {
        fprintf(stderr, "ERROR: Invalid row or column number!\n");
        exit(EXIT_FAILURE);
    }

    Node* current = (Node*)header_ptr;
    for (int column_ix = 0; column_ix < node_column; column_ix++)
        current = current->right;
    for (int row_ix = 0; row_ix < node_row; row_ix++)
        current = current->down;

    return current;
}

// Initialization functions:
Node* dlx_initiate_node(HeaderNode* column_header_ptr, int sudoku_row, int sudoku_column,
                        int sudoku_value)
{
    Node* node = malloc(sizeof(Node));
    node->sudoku_row = sudoku_row;
    node->sudoku_column = sudoku_column;
    node->sudoku_value = sudoku_value;
    node->column_node = column_header_ptr;
    return node;
}


// Functions for linking nodes vertically/horizontally:
void dlx_connect_nodes_vertical(Node* up, Node* down)
{
    if (up == NULL || down == NULL) return;
    up->down = down;
    down->up = up;
}

void dlx_connect_nodes_horizontal(Node* left, Node* right)
{
    if (left == NULL || right == NULL) return;
    left->right = right;
    right->left = left;
}


// Functions for getting DLX Matrix's column information:

HeaderNode* dlx_get_column_shortest(HeaderNode* header_ptr)
{
    HeaderNode* current = (HeaderNode*)header_ptr->node.right;
    HeaderNode* smallest = NULL;

    while (current != header_ptr)
    {
        if (smallest == NULL || current->current_size < smallest->current_size)
            smallest = current;

        current = (HeaderNode*)current->node.right;
    }

    return smallest;
}

HeaderNode* dlx_get_column(HeaderNode* header_ptr, int column_num)
{
    if (column_num < 1 || column_num > header_ptr->total_size)
    {
        fprintf(stderr, "ERROR: Invalid column number!\n");
        exit(EXIT_FAILURE);
    }

    HeaderNode* current = (HeaderNode*)header_ptr->node.right;
    while (current != header_ptr && current->num <= column_num)
    {
        if (current->num == column_num)
            return current;
        current = (HeaderNode*)current->node.right;
    }

    fprintf(stderr, "ERROR: Column header #%d not found!\n", column_num);
    return NULL;
}

Node* dlx_get_column_last_node(HeaderNode* column_ptr)
{
    Node* last_node = (Node*)&column_ptr->node;
    while (last_node->down != (Node*)column_ptr)
        last_node = last_node->down;

    return last_node;
}

// Functions for appending nodes/rows (!):
Node* dlx_append_node(HeaderNode* column_ptr, Node* column_last_node_ptr, int sudoku_row, int sudoku_column,
                      int sudoku_value)
{
    if (column_last_node_ptr == NULL || column_last_node_ptr->column_node != column_ptr)
        column_last_node_ptr = dlx_get_column_last_node(column_ptr);

    Node* new_node = dlx_initiate_node(column_ptr, sudoku_row, sudoku_column, sudoku_value);

    dlx_connect_nodes_vertical(new_node, column_last_node_ptr == (Node*)column_ptr
                                             ? (Node*)column_ptr
                                             : (Node*)column_last_node_ptr->column_node);
    dlx_connect_nodes_vertical(column_last_node_ptr, new_node);
    new_node->column_node->total_size++;
    new_node->column_node->current_size++;

    return new_node;
}

void dlx_append_row(HeaderNode* header_ptr, int sudoku_row, int sudoku_column, int sudoku_value)
{
    Node* new_node_first = NULL;
    Node* new_node_right = NULL;
    Node* new_node_left = NULL;
    Node* previous_row_node = dlx_get_column_last_node(dlx_get_column(header_ptr, 1));

    for (HeaderNode* column_ptr = (HeaderNode*)header_ptr->node.right;
         column_ptr != header_ptr; column_ptr = (HeaderNode*)column_ptr->node.right)
    {
        new_node_right = dlx_append_node(column_ptr, previous_row_node, sudoku_row, sudoku_column, sudoku_value);

        if (new_node_first != NULL)
            dlx_connect_nodes_horizontal(new_node_left, new_node_right);
        else
            new_node_first = new_node_right;

        new_node_left = new_node_right;
        previous_row_node = previous_row_node->right;
    }

    dlx_connect_nodes_horizontal(new_node_right, new_node_first);
}

// Functions for DLX Matrix initialization/termination:
HeaderNode* dlx_initialize_matrix_header_full(int column_num)
{
    if (column_num < 0)
    {
        fprintf(stderr, "ERROR: Number of columns must be non-negative!\n");
        exit(EXIT_FAILURE);
    }

    HeaderNode* header_ptr = (HeaderNode*)malloc(sizeof(HeaderNode));
    dlx_connect_nodes_vertical((Node*)header_ptr, (Node*)header_ptr);
    dlx_connect_nodes_horizontal((Node*)header_ptr, (Node*)header_ptr);
    header_ptr->node.column_node = header_ptr;
    header_ptr->total_size = column_num;
    header_ptr->current_size = column_num;
    header_ptr->num = 0;

    HeaderNode* previous_column_ptr = header_ptr;
    for (int column_ix = 1; column_ix <= column_num; column_ix++)
    {
        HeaderNode* current_column_ptr = (HeaderNode*)malloc(sizeof(HeaderNode));
        dlx_connect_nodes_vertical((Node*)current_column_ptr, (Node*)current_column_ptr);
        dlx_connect_nodes_horizontal((Node*)previous_column_ptr, (Node*)current_column_ptr);
        current_column_ptr->node.column_node = header_ptr;
        current_column_ptr->total_size = 0;
        current_column_ptr->current_size = 0;
        current_column_ptr->num = column_ix;
        previous_column_ptr = current_column_ptr;
    }

    dlx_connect_nodes_horizontal((Node*)previous_column_ptr, (Node*)header_ptr);
    return header_ptr;
}

// HeaderNode* dlx_initialize_sudoku_matrix_full(int sudoku_size)
// {
//     if (sudoku_size < 0)
//     {
//         fprintf(stderr, "ERROR: Sudoku size must be non-negative!\n");
//         exit(EXIT_FAILURE);
//     }
//
//     int sudoku_cells = (int)pow(sudoku_size, 2);
//     int matrix_columns = 4 * sudoku_cells;
//     int matrix_rows = sudoku_size * sudoku_cells;
//
//     HeaderNode* header = dlx_initialize_matrix_header_full(matrix_columns);
//     for (int matrix_row = 1; matrix_row <= matrix_rows; matrix_row++)
//     {
//         int sudoku_row = ((matrix_row - 1) / sudoku_cells) + 1;
//         int sudoku_column = (((matrix_row - 1) / sudoku_size) % sudoku_size) + 1;
//         int sudoku_value = ((matrix_row - 1) % (sudoku_size + 1)) + 1;
//
//         dlx_append_row(header, sudoku_row, sudoku_column, sudoku_value);
//     }
//
//     return header;
// }

HeaderNode* dlx_initialize_sudoku_matrix_compact(int sudoku_size)
{
    if (sudoku_size < 0)
    {
        fprintf(stderr, "ERROR: Sudoku size must be non-negative!\n");
        exit(EXIT_FAILURE);
    }

    int sudoku_cells = (int)pow(sudoku_size, 2);
    int sudoku_block_size = (int)sqrt(sudoku_size);
    int matrix_columns = 4 * sudoku_cells;
    int matrix_rows = sudoku_size * sudoku_cells;

    HeaderNode* header = dlx_initialize_matrix_header_full(matrix_columns);
    for (int matrix_row = 1; matrix_row <= matrix_rows; matrix_row++)
    {
        int sudoku_row = (matrix_row - 1) / sudoku_cells + 1;
        int sudoku_column = (matrix_row - 1) / sudoku_size % sudoku_size + 1;
        int sudoku_value = (matrix_row - 1) % sudoku_size + 1;;
        int sudoku_block = (sudoku_column - 1) / sudoku_block_size + ((sudoku_row - 1) / sudoku_block_size) *
            sudoku_block_size;

        int cell_constraint_column = (sudoku_row - 1) * sudoku_size + sudoku_column + (0 * sudoku_cells);
        int row_constraint_column = (sudoku_row - 1) * sudoku_size + sudoku_value + (1 * sudoku_cells);
        int column_constraint_column = (sudoku_column - 1) * sudoku_size + sudoku_value + (2 * sudoku_cells);
        int block_constraint_column = (sudoku_block) * sudoku_size + sudoku_value + (3 * sudoku_cells);

        // Uncomment for debugging purposes:
        //printf("[R: %d, C: %d, V: %d, B: %d -> C1: %3d, C2: %3d, C3: %3d, C4: %3d]\n",
        //       sudoku_row, sudoku_column, sudoku_value, sudoku_block,
        //       cell_constraint_column, row_constraint_column, column_constraint_column, block_constraint_column);

        HeaderNode* cell_constraint_column_ptr = dlx_get_column(header, cell_constraint_column);
        HeaderNode* row_constraint_column_ptr = dlx_get_column(header, row_constraint_column);
        HeaderNode* column_constraint_column_ptr = dlx_get_column(header, column_constraint_column);
        HeaderNode* block_constraint_column_ptr = dlx_get_column(header, block_constraint_column);

        Node* cell_constraint = dlx_append_node(cell_constraint_column_ptr, NULL, sudoku_row, sudoku_column,
                                                sudoku_value);
        Node* row_constraint = dlx_append_node(row_constraint_column_ptr, NULL, sudoku_row, sudoku_column,
                                               sudoku_value);
        Node* column_constraint = dlx_append_node(column_constraint_column_ptr, NULL, sudoku_row, sudoku_column,
                                                  sudoku_value);
        Node* block_constraint = dlx_append_node(block_constraint_column_ptr, NULL, sudoku_row, sudoku_column,
                                                 sudoku_value);

        dlx_connect_nodes_horizontal(cell_constraint, row_constraint);
        dlx_connect_nodes_horizontal(row_constraint, column_constraint);
        dlx_connect_nodes_horizontal(column_constraint, block_constraint);
        dlx_connect_nodes_horizontal(block_constraint, cell_constraint);
    }

    return header;
}

void dlx_terminate_matrix(HeaderNode* header_ptr)
{
    if (!header_ptr) return;

    for (HeaderNode* column_ptr = (HeaderNode*)header_ptr->node.right;
         column_ptr != header_ptr;)
    {
        for (Node* node_ptr = column_ptr->node.down;
             node_ptr != &(column_ptr->node);)
        {
            node_ptr = node_ptr->down;
            free((Node*)node_ptr->up);
        }
        column_ptr = (HeaderNode*)column_ptr->node.right;
        free((HeaderNode*)column_ptr->node.left);
    }
    free(header_ptr);
}

// Functions for checking the node's coverage status:
bool dlx_is_node_covered(Node* node_ptr)
{
    return !(node_ptr->up->down == node_ptr &&
        node_ptr->down->up == node_ptr);
}

bool dlx_is_column_covered(HeaderNode* column_ptr)
{
    return !(column_ptr->node.left->right == &(column_ptr->node) &&
        column_ptr->node.right->left == &(column_ptr->node));
}


// Functions for un-/covering nodes:
void dlx_uncover_node(Node* node_ptr)
{
    if (!dlx_is_node_covered(node_ptr)) return;
    dlx_connect_nodes_vertical(node_ptr->up, node_ptr);
    dlx_connect_nodes_vertical(node_ptr, node_ptr->down);
    node_ptr->column_node->current_size++;
}

void dlx_cover_node(Node* node_ptr)
{
    if (dlx_is_node_covered(node_ptr)) return;
    dlx_connect_nodes_vertical(node_ptr->up, node_ptr->down);
    node_ptr->column_node->current_size--;
}


// Functions for un-/covering column headers:
void dlx_uncover_column_header(HeaderNode* column_ptr)
{
    if (!dlx_is_column_covered(column_ptr)) return;
    dlx_connect_nodes_horizontal(column_ptr->node.left, &(column_ptr->node));
    dlx_connect_nodes_horizontal(&(column_ptr->node), column_ptr->node.right);
    column_ptr->node.column_node->current_size++;
}

void dlx_cover_column_header(HeaderNode* column_ptr)
{
    if (dlx_is_column_covered(column_ptr)) return;
    dlx_connect_nodes_horizontal(column_ptr->node.left, column_ptr->node.right);
    column_ptr->node.column_node->current_size--;
}


// Functions for un-/covering all nodes in a node's row, except the node itself:
void dlx_uncover_row(Node* node_ptr)
{
    for (Node* current = node_ptr->right; current != node_ptr; current = current->right)
        dlx_uncover_node(current);
}

void dlx_cover_row(Node* node_ptr)
{
    for (Node* current = node_ptr->right; current != node_ptr; current = current->right)
        dlx_cover_node(current);
}


// Functions for un-/covering all rows in the node's column and its column header:
void dlx_uncover_column(Node* node_ptr)
{
    HeaderNode* column_header_ptr = node_ptr->column_node;
    dlx_uncover_column_header(column_header_ptr);
    for (Node* current = node_ptr->down; current != node_ptr; current = current->down)
    {
        if (current == &(column_header_ptr->node)) continue;
        dlx_uncover_row(current);
    }
}

void dlx_cover_column(Node* node_ptr)
{
    HeaderNode* column_header_ptr = node_ptr->column_node;
    dlx_cover_column_header(node_ptr->column_node);
    for (Node* current = node_ptr->down; current != node_ptr; current = current->down)
    {
        if (current == &(column_header_ptr->node)) continue;
        dlx_cover_row(current);
    }
}

// Functions for in-/excluding node's row as part of the partial solution:
void dlx_include_row(Node* node_ptr)
{
    dlx_cover_column_header(node_ptr->column_node);
    for (Node* current = node_ptr->right; current != node_ptr; current = current->right)
        dlx_cover_column(current);
}

void dlx_exclude_row(Node* node_ptr)
{
    dlx_uncover_column_header(node_ptr->column_node);
    for (Node* current = node_ptr->right; current != node_ptr; current = current->right)
        dlx_uncover_column(current);
}

// Utility functions:
void dlx_print_node(HeaderNode* header_ptr, int node_row, int node_column)
{
    Node* node_ptr = dlx_get_node(header_ptr, node_row, node_column);
    printf("[Node: Row, Column, Value]\n-> N: %d, %d, %d\n\n", node_ptr->sudoku_row, node_ptr->sudoku_column,
           node_ptr->sudoku_value);
}

void dlx_print_header(HeaderNode* header_ptr)
{
    printf("[Matrix: Rows, Columns]\n-> H0: %d, %d\n\n", ((HeaderNode*)header_ptr->node.right)->total_size,
           header_ptr->total_size);
}

void dlx_print_sizes(HeaderNode* header_ptr)
{
    HeaderNode* current = header_ptr;

    printf("[Column: Column size]\n");
    for (int ix = 0; ix <= header_ptr->current_size; ix++)
    {
        printf("-> %s%3d: %d\n",
               (current->num > 0) ? "C" : "H",
               current->num,
               current->current_size);

        current = (HeaderNode*)current->node.right;
    }
    printf("\n");
}

// Sudoku functions:
Node* dlx_set_constraint(HeaderNode* header_ptr, int sudoku_size, int sudoku_row, int sudoku_column, int sudoku_value)
{
    int column_num = (sudoku_row - 1) * sudoku_size + sudoku_column;
    HeaderNode* column_ptr = dlx_get_column(header_ptr, column_num);
    if (column_ptr == NULL)
    {
        fprintf(stderr, "Constraint not set! (column not found)\n");
        return NULL;
    }

    for (Node* node_ptr = column_ptr->node.down; node_ptr != &(column_ptr->node); node_ptr = node_ptr->down)
    {
        if (node_ptr->sudoku_row == sudoku_row && node_ptr->sudoku_column == sudoku_column &&
            node_ptr->sudoku_value == sudoku_value)
        {
            dlx_include_row(node_ptr);
            printf("Constraint set!\n");
            return node_ptr;
        }
    }

    fprintf(stderr, "Constraint not set! (node not found)\n");
    return NULL;
}

bool dlx_solve(HeaderNode* header_ptr, int* sudoku_grid, int sudoku_size)
{
    HeaderNode* selected_column_ptr = dlx_get_column_shortest(header_ptr);
    if (selected_column_ptr == NULL)
    {
        // TODO: implement proper result handling.
        printf("[SUCCS] #%d Solution found!\n\n\n", ++test);
        test = 0;
        return true;
    }

    for (Node* selected_node_ptr = selected_column_ptr->node.down;
         selected_node_ptr != &(selected_column_ptr->node);
         selected_node_ptr = selected_node_ptr->down)
    {
        dlx_include_row(selected_node_ptr);
        if (dlx_solve(header_ptr, sudoku_grid, sudoku_size))
        {
            int selected_node_row_index = selected_node_ptr->sudoku_row - 1;
            int selected_node_column_index = selected_node_ptr->sudoku_column - 1;
            int selected_node_value = selected_node_ptr->sudoku_value;

            int digits_index = selected_node_row_index * sudoku_size + selected_node_column_index;
            sudoku_grid[digits_index] = selected_node_value;

            // Optional if the original, unmodified version of the matrix is required:
            dlx_exclude_row(selected_node_ptr);

            return true;
        }

        dlx_exclude_row(selected_node_ptr);
    }
    printf("[FAILD] #%d No solution found!\n", ++test);
    return false;
}

// [TEMP] Temporary code, used exclusively for testing. This code will be removed later.
void grid_print_border_horizontal(int sudoku_size)
{
    for (int ix = 0; ix < sudoku_size; ix++)
    {
        bool add_border_vertical = ((ix + 1) % (int)sqrt(sudoku_size) == 0);
        bool end_row = ((ix + 1) % sudoku_size == 0);
        printf("-%s", (end_row) ? "" : "-");

        if (add_border_vertical && !end_row)
            printf("+-");
        if (end_row)
            printf("\n");
    }
}

void grid_print(int* sudoku_grid, int sudoku_size)
{
    for (int ix = 0; ix < (int)pow(sudoku_size, 2); ix++)
    {
        bool add_border_vertical = ((ix + 1) % (int)sqrt(sudoku_size) == 0);
        bool add_border_horizontal = ((ix + 1) % (sudoku_size * (int)sqrt(sudoku_size)) == 0);
        bool end_row = ((ix + 1) % sudoku_size == 0);
        bool last_row = end_row && (ix == (int)pow(sudoku_size, 2) - 1);

        printf("%d%s", sudoku_grid[ix], (end_row) ? "" : " ");

        if (add_border_vertical && !end_row)
            printf("| ");
        if (end_row)
            printf("\n");
        if (add_border_horizontal && !last_row)
            grid_print_border_horizontal(sudoku_size);
    }
}

int main()
{
    int sudoku_size = 9;
    int* sudoku_grid = (int*)calloc((int)pow(sudoku_size, 2), sizeof(int));
    HeaderNode* header_ptr = dlx_initialize_sudoku_matrix_compact(sudoku_size);

    dlx_set_constraint(header_ptr, sudoku_size, 3, 1, 5);

    dlx_solve(header_ptr, sudoku_grid, sudoku_size);

    grid_print(sudoku_grid, sudoku_size);

    dlx_terminate_matrix(header_ptr);
    return 0;
}
