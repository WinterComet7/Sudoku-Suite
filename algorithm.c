#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Node Node;
typedef struct HeaderNode HeaderNode;

struct Node
{
    int sudoku_row, sudoku_column, sudoku_value;
    Node *left, *right, *up, *down;
    HeaderNode* header_node;
};

struct HeaderNode
{
    Node node;
    int total_size; // Total number of children nodes, both covered and uncovered.
    int current_size; // Number of children nodes in the column that are currently uncovered.
    int num;
};

/**
 * @brief Retrieves a specific node from a Dancing Links (DLX) matrix given its row and column indices.
 *
 * This function navigates through the DLX matrix starting from the given header node (header_ptr).
 * It first moves to the desired column by following the 'right' pointers and then moves to the desired
 * row by following the 'down' pointers. If the provided row or column indices are invalid, the program
 * exits with an error message.
 *
 * @param header_ptr Pointer to the header node of the DLX column from which navigation begins.
 *                   This should represent the starting point for column and row traversal.
 * @param matrix_row_index The index of the row in the DLX matrix to retrieve the node from. Must be non-negative.
 * @param matrix_column_index The index of the column in the DLX matrix to retrieve the node from.
 *                            Must be non-negative and not greater than the total size of the header node.
 *
 * @return Pointer to the node located at the specified row and column in the DLX matrix.
 *         The node is identified based on traversal from the header node.
 *
 * @note The indices are zero-based.
 * @note Exits the program with an error message if the row or column indices are out of bounds.
 */
Node* dlx_get_node(HeaderNode* header_ptr, int matrix_row_index, int matrix_column_index)
{
    // TODO: implement validity checks.

    if (matrix_row_index < 0 || matrix_column_index < 0 || matrix_column_index > header_ptr->total_size)
    {
        fprintf(stderr, "ERROR: Invalid row or column number!\n");
        exit(EXIT_FAILURE);
    }

    Node* current_node_ptr = (Node*)header_ptr;
    for (int column_index = 0; column_index < matrix_column_index; column_index++)
        current_node_ptr = current_node_ptr->right;
    for (int row_index = 0; row_index < matrix_row_index; row_index++)
        current_node_ptr = current_node_ptr->down;

    return current_node_ptr;
}


/**
 * @brief Links two nodes vertically within the Dancing Links (DLX) matrix.
 *
 * This function connects two nodes vertically by setting the 'down' pointer of the upper node
 * to point to the lower node, and the 'up' pointer of the lower node to point to the upper node.
 * If either of the nodes is NULL, the operation is skipped to ensure safety.
 *
 * @param up Pointer to the upper node to be linked.
 * @param down Pointer to the lower node to be linked.
 *
 * @note The function assumes that both nodes reside in the same column of the DLX matrix.
 *       It does not perform any validity checks regarding nodes' column memberships.
 */
void dlx_link_nodes_vertical(Node* up, Node* down)
{
    if (up == NULL || down == NULL) return;
    up->down = down;
    down->up = up;
}

/**
 * @brief Links two nodes horizontally within the Dancing Links (DLX) matrix.
 *
 * This function connects two nodes vertically by setting the 'right' pointer of the left node
 * to point to the right node, and the 'left' pointer of the right node to point to the left node.
 * If either of the nodes is NULL, the operation is skipped to ensure safety.
 *
 * @param left Pointer to the left node to be linked.
 * @param right Pointer to the right node to be linked.
 *
 * @note The function assumes that both nodes reside in the same column of the DLX matrix.
 *       It does not perform any validity checks regarding nodes' column memberships.
 */
void dlx_link_nodes_horizontal(Node* left, Node* right)
{
    if (left == NULL || right == NULL) return;
    left->right = right;
    right->left = left;
}


/**
 * @brief Finds the column with the smallest number of uncovered nodes in a Dancing Links (DLX) matrix.
 *
 * This function iterates through all header nodes starting from the given header node (header_ptr)
 * and identifies the column with the smallest `current_size` (number of currently uncovered nodes).
 * If no columns are found, it returns NULL.
 *
 * @param header_ptr Pointer to the header node representing the starting point for column traversal.
 *                   This node's 'right' and 'left' pointers form a circular linked list of header nodes.
 *
 * @return Pointer to the header node corresponding to the column with the smallest `current_size`.
 *         Returns NULL if no valid column is found.
 *
 * @note Assumes non-empty DLX matrix traversal. The provided header node is expected to be part of the circular
 *       linked list of header nodes.
 */
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

HeaderNode* dlx_get_column(HeaderNode* header_ptr, int matrix_column_index)
{
    if (matrix_column_index < 1 || matrix_column_index > header_ptr->total_size)
    {
        fprintf(stderr, "ERROR: Invalid column number!\n");
        exit(EXIT_FAILURE);
    }

    HeaderNode* current = (HeaderNode*)header_ptr->node.right;
    while (current != header_ptr && current->num <= matrix_column_index)
    {
        if (current->num == matrix_column_index)
            return current;
        current = (HeaderNode*)current->node.right;
    }

    fprintf(stderr, "ERROR: Column header #%d not found!\n", matrix_column_index);
    return NULL;
}

Node* dlx_get_column_last_node(HeaderNode* column_ptr)
{
    Node* last_node = &(column_ptr->node);
    while (last_node->down != (Node*)column_ptr)
        last_node = last_node->down;

    return last_node;
}

// Functions for appending nodes/rows (!):
Node* dlx_append_node(HeaderNode* column_ptr, Node* column_last_node_ptr, int sudoku_row, int sudoku_column,
                      int sudoku_value)
{
    if (column_last_node_ptr == NULL || column_last_node_ptr->header_node != column_ptr)
        column_last_node_ptr = dlx_get_column_last_node(column_ptr);

    // Initiate new node:
    Node* new_node = malloc(sizeof(Node));
    new_node->sudoku_row = sudoku_row;
    new_node->sudoku_column = sudoku_column;
    new_node->sudoku_value = sudoku_value;

    // Append the node to the column:
    new_node->header_node = column_ptr;
    new_node->header_node->total_size++;
    new_node->header_node->current_size++;
    dlx_link_nodes_vertical(new_node, &(column_ptr->node));
    dlx_link_nodes_vertical(column_last_node_ptr, new_node);

    return new_node;
}

// Functions for DLX Matrix initialization/termination:
HeaderNode* dlx_initialize_matrix_header_full(int num_columns)
{
    if (num_columns < 0)
    {
        fprintf(stderr, "ERROR: Number of columns must be non-negative!\n");
        exit(EXIT_FAILURE);
    }

    HeaderNode* header_ptr = (HeaderNode*)malloc(sizeof(HeaderNode));
    dlx_link_nodes_vertical((Node*)header_ptr, (Node*)header_ptr);
    dlx_link_nodes_horizontal((Node*)header_ptr, (Node*)header_ptr);
    header_ptr->node.header_node = header_ptr;
    header_ptr->total_size = num_columns;
    header_ptr->current_size = num_columns;
    header_ptr->num = 0;

    HeaderNode* previous_column_ptr = header_ptr;
    for (int matrix_column_index = 1; matrix_column_index <= num_columns; matrix_column_index++)
    {
        HeaderNode* current_column_ptr = (HeaderNode*)malloc(sizeof(HeaderNode));
        dlx_link_nodes_vertical((Node*)current_column_ptr, (Node*)current_column_ptr);
        dlx_link_nodes_horizontal((Node*)previous_column_ptr, (Node*)current_column_ptr);
        current_column_ptr->node.header_node = current_column_ptr;
        current_column_ptr->total_size = 0;
        current_column_ptr->current_size = 0;
        current_column_ptr->num = matrix_column_index;
        previous_column_ptr = current_column_ptr;
    }

    dlx_link_nodes_horizontal((Node*)previous_column_ptr, (Node*)header_ptr);
    return header_ptr;
}

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

    HeaderNode* header_ptr = dlx_initialize_matrix_header_full(matrix_columns);
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

        HeaderNode* cell_constraint_column_ptr = dlx_get_column(header_ptr, cell_constraint_column);
        HeaderNode* row_constraint_column_ptr = dlx_get_column(header_ptr, row_constraint_column);
        HeaderNode* column_constraint_column_ptr = dlx_get_column(header_ptr, column_constraint_column);
        HeaderNode* block_constraint_column_ptr = dlx_get_column(header_ptr, block_constraint_column);

        Node* cell_constraint = dlx_append_node(cell_constraint_column_ptr, NULL, sudoku_row, sudoku_column,
                                                sudoku_value);
        Node* row_constraint = dlx_append_node(row_constraint_column_ptr, NULL, sudoku_row, sudoku_column,
                                               sudoku_value);
        Node* column_constraint = dlx_append_node(column_constraint_column_ptr, NULL, sudoku_row, sudoku_column,
                                                  sudoku_value);
        Node* block_constraint = dlx_append_node(block_constraint_column_ptr, NULL, sudoku_row, sudoku_column,
                                                 sudoku_value);

        dlx_link_nodes_horizontal(cell_constraint, row_constraint);
        dlx_link_nodes_horizontal(row_constraint, column_constraint);
        dlx_link_nodes_horizontal(column_constraint, block_constraint);
        dlx_link_nodes_horizontal(block_constraint, cell_constraint);
    }

    return header_ptr;
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


// Functions for un-/covering nodes:
void dlx_uncover_node(Node* node_ptr)
{
    dlx_link_nodes_vertical(node_ptr->up, node_ptr);
    dlx_link_nodes_vertical(node_ptr, node_ptr->down);
    node_ptr->header_node->current_size++;
}

void dlx_cover_node(Node* node_ptr)
{
    dlx_link_nodes_vertical(node_ptr->up, node_ptr->down);
    node_ptr->header_node->current_size--;
}


// Functions for un-/covering column headers:
void dlx_uncover_column_header(HeaderNode* header_ptr, HeaderNode* column_ptr)
{
    dlx_link_nodes_horizontal(column_ptr->node.left, &(column_ptr->node));
    dlx_link_nodes_horizontal(&(column_ptr->node), column_ptr->node.right);
    header_ptr->current_size++;
}

void dlx_cover_column_header(HeaderNode* header_ptr, HeaderNode* column_ptr)
{
    dlx_link_nodes_horizontal(column_ptr->node.left, column_ptr->node.right);
    header_ptr->current_size--;
}


// Functions for ex-/including nodes as a part of the solution:
void dlx_uncover_affected_nodes(HeaderNode* header_ptr, HeaderNode** constraint_column_ptrs,
                                int constraint_column_ptrs_length)
{
    for (int index = 0; index < constraint_column_ptrs_length; index++)
    {
        HeaderNode* constraint_column_ptr = constraint_column_ptrs[index];
        dlx_uncover_column_header(header_ptr, constraint_column_ptr);

        // Traverse down the covered column.
        for (Node* column_node_ptr = constraint_column_ptr->node.down; column_node_ptr != &(constraint_column_ptr->node)
             ; column_node_ptr = column_node_ptr->down)
        {
            // Traverse one of the affected rows.
            for (Node* row_node_ptr = column_node_ptr->right; row_node_ptr != column_node_ptr; row_node_ptr =
                 row_node_ptr->right)
            {
                // Check if the neighboring node is in any of the columns being covered.
                for (int check_index = 0; check_index < constraint_column_ptrs_length; check_index++)
                {
                    if (row_node_ptr->header_node == constraint_column_ptrs[check_index]) break;
                }

                // If the neighboring node is not in any of the columns being covered, cover it.
                dlx_uncover_node(row_node_ptr);
            }
        }
    }
}

void dlx_cover_affected_nodes(HeaderNode* header_ptr, HeaderNode** constraint_column_ptrs,
                              int constraint_column_ptrs_length)
{
    for (int index = 0; index < constraint_column_ptrs_length; index++)
    {
        HeaderNode* constraint_column_ptr = constraint_column_ptrs[index];
        dlx_cover_column_header(header_ptr, constraint_column_ptr);

        // Traverse down the covered column.
        for (Node* column_node_ptr = constraint_column_ptr->node.down; column_node_ptr != &(constraint_column_ptr->node)
             ; column_node_ptr = column_node_ptr->down)
        {
            // Traverse one of the affected rows.
            for (Node* row_node_ptr = column_node_ptr->right; row_node_ptr != column_node_ptr; row_node_ptr =
                 row_node_ptr->right)
            {
                // Check if the neighboring node is in any of the columns being covered.
                for (int check_index = 0; check_index < constraint_column_ptrs_length; check_index++)
                {
                    if (row_node_ptr->header_node == constraint_column_ptrs[check_index]) break;
                }

                // If the neighboring node is not in any of the columns being covered, cover it.
                dlx_cover_node(row_node_ptr);
            }
        }
    }
}

void dlx_include_node(HeaderNode* header_ptr, Node* node_ptr, int sudoku_size)
{
    int sudoku_cells = (int)pow(sudoku_size, 2);
    int sudoku_block_size = (int)sqrt(sudoku_size);

    int sudoku_row = node_ptr->sudoku_row;
    int sudoku_column = node_ptr->sudoku_column;
    int sudoku_value = node_ptr->sudoku_value;
    int sudoku_block = (sudoku_column - 1) / sudoku_block_size + ((sudoku_row - 1) / sudoku_block_size) *
        sudoku_block_size;

    int cell_constraint_column = (sudoku_row - 1) * sudoku_size + sudoku_column + (0 * sudoku_cells);
    int row_constraint_column = (sudoku_row - 1) * sudoku_size + sudoku_value + (1 * sudoku_cells);
    int column_constraint_column = (sudoku_column - 1) * sudoku_size + sudoku_value + (2 * sudoku_cells);
    int block_constraint_column = (sudoku_block) * sudoku_size + sudoku_value + (3 * sudoku_cells);

    int constraint_column_ptrs_length = 4;

    HeaderNode* cell_constraint_column_ptr = dlx_get_column(header_ptr, cell_constraint_column);
    HeaderNode* row_constraint_column_ptr = dlx_get_column(header_ptr, row_constraint_column);
    HeaderNode* column_constraint_column_ptr = dlx_get_column(header_ptr, column_constraint_column);
    HeaderNode* block_constraint_column_ptr = dlx_get_column(header_ptr, block_constraint_column);

    HeaderNode** constraint_column_ptrs = (HeaderNode**)malloc(sizeof(HeaderNode*) * constraint_column_ptrs_length);
    constraint_column_ptrs[0] = cell_constraint_column_ptr;
    constraint_column_ptrs[1] = row_constraint_column_ptr;
    constraint_column_ptrs[2] = column_constraint_column_ptr;
    constraint_column_ptrs[3] = block_constraint_column_ptr;

    dlx_cover_affected_nodes(header_ptr, constraint_column_ptrs, constraint_column_ptrs_length);
    free(constraint_column_ptrs);
}

void dlx_exclude_node(HeaderNode* header_ptr, Node* node_ptr, int sudoku_size)
{
    int constraint_column_ptrs_length = 4;

    HeaderNode* constraint_column_ptr_1 = node_ptr->header_node;
    HeaderNode* constraint_column_ptr_2 = node_ptr->right->header_node;
    HeaderNode* constraint_column_ptr_3 = node_ptr->right->right->header_node;
    HeaderNode* constraint_column_ptr_4 = node_ptr->right->right->right->header_node;

    HeaderNode** constraint_column_ptrs = (HeaderNode**)malloc(sizeof(HeaderNode*) * constraint_column_ptrs_length);
    constraint_column_ptrs[0] = constraint_column_ptr_1;
    constraint_column_ptrs[1] = constraint_column_ptr_2;
    constraint_column_ptrs[2] = constraint_column_ptr_3;
    constraint_column_ptrs[3] = constraint_column_ptr_4;

    dlx_uncover_affected_nodes(header_ptr, constraint_column_ptrs, constraint_column_ptrs_length);
    free(constraint_column_ptrs);
}


// Utility functions:
void dlx_print_node(Node* node_ptr)
{
    printf("[r%dc%d#%d]", node_ptr->sudoku_row, node_ptr->sudoku_column, node_ptr->sudoku_value);
}

void dlx_print_matrix_compact(HeaderNode* header_ptr)
{
    HeaderNode* current = header_ptr;

    printf("[COMP output]\n");
    for (int ix = 0; ix <= header_ptr->current_size; ix++)
    {
        printf("| %s%3d: %d |\n",
               (current->num > 0) ? "C" : "H",
               current->num,
               current->current_size);

        current = (HeaderNode*)current->node.right;
    }
    printf("\n");
}

void dlx_print_matrix_full(HeaderNode* header_ptr)
{
    HeaderNode* current = header_ptr;
    printf("[FULL output]\n");
    for (int ix = 0; ix <= header_ptr->current_size; ix++)
    {
        printf("| %s%3d: %3d |",
               (current->num > 0) ? "C" : "H",
               current->num,
               current->current_size);

        if (current->num > 0)
        {
            for (Node* node_ptr = current->node.down; node_ptr != &(current->node); node_ptr = node_ptr->down)
            {
                printf(" > ");
                dlx_print_node(node_ptr);
            }
        }

        printf("\n");
        current = (HeaderNode*)current->node.right;
    }
}

// Sudoku functions:
Node* dlx_set_constraint(HeaderNode* header_ptr, int* sudoku_grid, int sudoku_size, int sudoku_row, int sudoku_column,
                         int sudoku_value)
{
    if (sudoku_row < 1 || sudoku_row > sudoku_size ||
        sudoku_column < 1 || sudoku_column > sudoku_size ||
        sudoku_value < 1 || sudoku_value > sudoku_size)
    {
        fprintf(stderr, "Constraint r%dc%d#%d not set! (value out of bounds)\n", sudoku_row, sudoku_column,
                sudoku_value);
        return NULL;
    }

    int matrix_column_index = (sudoku_row - 1) * sudoku_size + sudoku_column;
    HeaderNode* column_ptr = dlx_get_column(header_ptr, matrix_column_index);
    if (column_ptr == NULL)
    {
        fprintf(stderr, "Constraint r%dc%d#%d not set! (column not found)\n", sudoku_row, sudoku_column, sudoku_value);
        return NULL;
    }

    for (Node* node_ptr = column_ptr->node.down; node_ptr != &(column_ptr->node); node_ptr = node_ptr->down)
    {
        if (node_ptr->sudoku_value == sudoku_value)
        {
            dlx_include_node(header_ptr, node_ptr, sudoku_size);
            int grid_index = (sudoku_row - 1) * sudoku_size + sudoku_column - 1;
            sudoku_grid[grid_index] = sudoku_value;

            printf("Constraint r%dc%d#%d set!\n", sudoku_row, sudoku_column, sudoku_value);
            return node_ptr;
        }
    }

    fprintf(stderr, "Constraint r%dc%d#%d not set! (node not found in column)\n", sudoku_row, sudoku_column,
            sudoku_value);
    return NULL;
}

bool dlx_solve(HeaderNode* header_ptr, int* sudoku_grid, int sudoku_size)
{
    HeaderNode* selected_column_ptr = dlx_get_column_shortest(header_ptr);
    if (header_ptr->current_size == 0 || selected_column_ptr == NULL)
    {
        // TODO: implement proper result handling.
        return true;
    }

    if (selected_column_ptr->current_size < 1) return false;

    // Uncomment for debugging purposes:
    // dlx_print_matrix_full(header_ptr);

    for (Node* selected_node_ptr = selected_column_ptr->node.down;
         selected_node_ptr != &(selected_column_ptr->node);
         selected_node_ptr = selected_node_ptr->down)
    {
        // Uncomment for debugging purposes:
        // printf("[PCKD: C%3d ]", selected_column_ptr->num);
        // dlx_print_node(selected_node_ptr);
        // printf("\n\n");

        dlx_include_node(header_ptr, selected_node_ptr, sudoku_size);
        if (dlx_solve(header_ptr, sudoku_grid, sudoku_size))
        {
            int selected_node_row_index = selected_node_ptr->sudoku_row - 1;
            int selected_node_column_index = selected_node_ptr->sudoku_column - 1;
            int selected_node_value = selected_node_ptr->sudoku_value;

            int digits_index = selected_node_row_index * sudoku_size + selected_node_column_index;
            sudoku_grid[digits_index] = selected_node_value;

            return true;
        }
        dlx_exclude_node(header_ptr, selected_node_ptr, sudoku_size);
    }

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
    // Basic test:

    int sudoku_size = 9;
    int* sudoku_grid = (int*)calloc((int)pow(sudoku_size, 2), sizeof(int));
    HeaderNode* header_ptr = dlx_initialize_sudoku_matrix_compact(sudoku_size);

    // Uncomment for debugging purposes:
    // dlx_print_matrix_full(header_ptr);
    // printf("\n");

    dlx_set_constraint(header_ptr, sudoku_grid, sudoku_size, 1, 1, 1);
    dlx_set_constraint(header_ptr, sudoku_grid, sudoku_size, 2, 2, 2);
    dlx_set_constraint(header_ptr, sudoku_grid, sudoku_size, 3, 3, 3);
    dlx_set_constraint(header_ptr, sudoku_grid, sudoku_size, 4, 4, 4);
    dlx_set_constraint(header_ptr, sudoku_grid, sudoku_size, 5, 5, 5);
    dlx_set_constraint(header_ptr, sudoku_grid, sudoku_size, 6, 6, 6);
    dlx_set_constraint(header_ptr, sudoku_grid, sudoku_size, 7, 7, 7);
    dlx_set_constraint(header_ptr, sudoku_grid, sudoku_size, 8, 8, 8);
    dlx_set_constraint(header_ptr, sudoku_grid, sudoku_size, 9, 9, 9);

    dlx_solve(header_ptr, sudoku_grid, sudoku_size);

    // Uncomment for debugging purposes:
    // dlx_print_matrix_full(header_ptr);
    // printf("\n");

    printf("Solution:\n");
    grid_print(sudoku_grid, sudoku_size);

    free(sudoku_grid);
    dlx_terminate_matrix(header_ptr);
    return 0;
}
