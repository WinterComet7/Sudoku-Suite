#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sudoku_algorithm.h"
#include "sudoku_main.h"

#define ENABLE_DEBUG_MODE false

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
 *  @brief Various utility functions intended for printing values, especially when debugging.
 */
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

/**
 *  @brief Various utility functions, intended for RNG.
 */

int random_interval(int min_value, int max_value)
{
    return rand() % (max_value - min_value + 1) + min_value;
}

/**
 * @brief Frees the memory allocated for a Dancing Links (DLX) matrix, including all nodes.
 *
 * This function safely deallocates the memory for all nodes within a given DLX matrix, starting from the
 * header node and navigating through all header and child nodes. It iterates over all columns and their
 * respective child nodes and frees the allocated memory. After completing the process, the memory used
 * by the header node is also freed.
 *
 * @param header_ptr Pointer to the starting header node of the DLX matrix. If the pointer is NULL, the
 *                   function exits without performing any operations. Otherwise, it navigates through
 *                   all columns and their children to release allocated memory.
 */
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

/**
 * @brief Retrieves a specific node from a Dancing Links (DLX) matrix given its row and column indices.
 *
 * This function navigates through the DLX matrix starting from the given matrix's header node (header_ptr).
 * It first moves to the desired column by following the 'right' pointers and then moves to the desired
 * row by following the 'down' pointers. If the provided row or column indices are invalid, the program
 * exits with an error message.
 *
 * @param header_ptr Pointer to the starting header node of the DLX matrix, representing the starting point
 *                   for column traversal. This node's 'right' and 'left' pointers form a circular
 *                   linked list of header nodes.
 * @param matrix_row_index The index of the row in the DLX matrix to retrieve the node from. Must be non-negative.
 * @param matrix_column_index The index of the column in the DLX matrix to retrieve the node from.
 *                            Must be non-negative and not greater than the total size of the header node.
 *
 * @return Pointer to the node located at the specified row and column in the DLX matrix.
 *         The node is identified based on traversal from the header node.
 *
 * @note The indices are zero-based (the matrix's header node being located at row index 0 and column index 0).
 * @note Exits the program with an error message if the row or column indices are out of bounds.
 */
Node* dlx_get_node(HeaderNode* header_ptr, int matrix_row_index, int matrix_column_index)
{
    // TODO: implement validity checks.

    if (matrix_row_index < 0 || matrix_column_index < 0 || matrix_column_index > header_ptr->total_size)
    {
        if (ENABLE_DEBUG_MODE) fprintf(stderr, "[ERR msg] Invalid row and/or column index!\n");
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
 * @param header_ptr Pointer to the starting header node of the DLX matrix, representing the starting point
 *                   for column traversal. This node's 'right' and 'left' pointers form a circular
 *                   linked list of header nodes.
 *
 * @return Pointer to the header node corresponding to the column with the smallest `current_size`.
 *         Returns NULL if no valid column is found or the DLX matrix has no header nodes, besides the
 *         starting header node of the DLX matrix.
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

/**
 * @brief Retrieves the header node for a specific column in a Dancing Links (DLX) matrix.
 *
 * This function traverses the circular doubly linked list of column headers starting from the
 * specified header node. It locates and returns the header node corresponding to the column
 * with the given index. If the column index is invalid or the column is not found, an error
 * message is displayed, and the function either terminates the program or returns NULL.
 *
 * @param header_ptr Pointer to the starting header node of the DLX matrix, representing the starting point
 *                   for column traversal. This node's 'right' and 'left' pointers form a circular
 *                   linked list of header nodes.
 *
 * @param matrix_column_index The one-based index of the column whose header node is to be retrieved.
 *                            Must be between 1 and the total size of the header node.
 *
 * @return Pointer to the header node of the specified column if found.
 *         Returns NULL if the column is not found.
 *
 * @note The indices are one-based, and the function will terminate the program with an error
 *       message if the column index is invalid (less than 1 or greater than the header node's total size).
 */
HeaderNode* dlx_get_column(HeaderNode* header_ptr, int matrix_column_index)
{
    if (matrix_column_index < 1 || matrix_column_index > header_ptr->total_size)
    {
        if (ENABLE_DEBUG_MODE)fprintf(stderr, "[ERR msg] Invalid column index!\n");
        exit(EXIT_FAILURE);
    }

    HeaderNode* current = (HeaderNode*)header_ptr->node.right;
    while (current != header_ptr && current->num <= matrix_column_index)
    {
        if (current->num == matrix_column_index)
            return current;
        current = (HeaderNode*)current->node.right;
    }

    if (ENABLE_DEBUG_MODE)fprintf(stderr, "[ERR msg] Column header #%d not found!\n", matrix_column_index);
    return NULL;
}

/**
 * @brief Retrieves the last node in a specific column of a Dancing Links (DLX) matrix.
 *
 * This function traverses a given column in the DLX matrix, starting from the column's header node,
 * to find the last node. The traversal proceeds by following the `down` pointers in a circular
 * linked list until it reaches the header node again, which marks the end of the column.
 *
 * @param column_ptr Pointer to the header node of the column whose last node needs to be retrieved.
 *                   The header node represents the starting point for the traversal.
 *
 * @return Pointer to the last node in the specified column of the DLX matrix. The returned node is
 *         the last one before looping back to the column's header node.
 *
 * @note Assumes that the column contains at least one node, which may include the header node itself.
 */
Node* dlx_get_column_last_node(HeaderNode* column_ptr)
{
    Node* last_node = &(column_ptr->node);
    while (last_node->down != (Node*)column_ptr)
        last_node = last_node->down;

    return last_node;
}

/**
 * @brief Appends a new node to a specified column in the Dancing Links (DLX) matrix.
 *
 * This function creates a new node and appends it vertically to the provided column in the DLX matrix.
 * The new node becomes part of the specified column's linked list, and the header node's size attributes
 * are updated accordingly. It ensures that the new node is linked appropriately to maintain the DLX matrix structure.
 *
 * @param column_ptr Pointer to the header node of the column where the new node will be appended. It represents
 *                   the starting point for modifications in the column.
 * @param column_last_node_ptr Pointer to the last known node in the column. If this pointer is incorrect or NULL,
 *                             the last node is retrieved dynamically using dlx_get_column_last_node.
 * @param sudoku_row The row number of the Sudoku puzzle that the new node represents.
 * @param sudoku_column The column number of the Sudoku puzzle that the new node represents.
 * @param sudoku_value The value of the Sudoku cell that the new node represents.
 *
 * @return Pointer to the newly created and appended node in the DLX matrix. The node is linked both to its
 *         header and vertically to the other nodes of the column.
 *
 * @note The function assumes memory allocation for the new node is successful. The caller must ensure to
 *       manage memory and free the node if necessary.
 * @note The function only links new nodes vertically. The horizontal linking has to be done manually afterward.
 *
 */
Node* dlx_append_node(HeaderNode* column_ptr, Node* column_last_node_ptr, int sudoku_row, int sudoku_column, int sudoku_value)
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

Node* dlx_set_constraint(HeaderNode* header_ptr, int* sudoku_grid, int sudoku_size, int sudoku_row, int sudoku_column, int sudoku_value);

/**
 * @brief Initializes the starting header node of a Dancing Links (DLX) matrix and creates
 *        its associated column headers.
 *
 * This function dynamically allocates memory for the starting header node representing
 * the DLX matrix and links all column header nodes in a circular doubly linked list structure.
 * Each column header node is initialized with default values, and the total header node
 * structure is prepared for further row additions.
 *
 * @param num_columns The total number of columns in the DLX matrix. Must be non-negative.
 *
 * @return Pointer to the header node of the initialized DLX matrix. This header node links to
 *         all column headers through a circular doubly linked list structure.
 *
 * @note If the provided number of columns is negative, the program will terminate with an error.
 * @note This function appropriately sets the values of header nodes' `total_size` and `current_size`
 *       attributes. These attributes are set to num_columns for the starting header node
 *       and 0 for all other header nodes.
 */
HeaderNode* dlx_initialize_matrix_header_full(int num_columns)
{
    if (num_columns < 0)
    {
        if (ENABLE_DEBUG_MODE)
            fprintf(
                stderr, "[ERR msg] Received negative number of columns %d! (number of columns must be non-negative)\n",
                num_columns);
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

/**
 * @brief Initializes a compact Sudoku matrix as a Dancing Links (DLX) matrix.
 *
 * This function creates a DLX matrix tailored to a Sudoku puzzle of the specified size
 * and returns a pointer to its header node. The matrix is constructed to include
 * all constraints required to solve the Sudoku puzzle (cell, row, column, and block constraints).
 *
 * The Sudoku size should represent the width/height of one block squared. For example,
 * a standard 9x9 Sudoku puzzle has `sudoku_size` set to 9.
 *
 * @param sudoku_size The size of the Sudoku puzzle. Must be non-negative. This represents
 *                    the dimensions of the grid (e.g., 9 for a 9x9 puzzle).
 *
 * @return Pointer to the header node of the initialized DLX sudoku matrix structure.
 *         The matrix represents all potential constraints for solving the Sudoku.
 *
 * @note The function will terminate the program with an error message if the provided
 *       `sudoku_size` is negative.
 */
HeaderNode* dlx_initialize_sudoku_matrix_compact(int sudoku_size)
{
    if (sudoku_size < 0)
    {
        if (ENABLE_DEBUG_MODE)
            fprintf(stderr, "[ERR msg] Received negative Sudoku size %d! (Sudoku size must be non-negative)\n",
                    sudoku_size);
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

// TODO: Add description.
HeaderNode* dlx_initialize_sudoku_matrix_from_grid(int* sudoku_grid, int sudoku_size)
{
    if (sudoku_grid == NULL || sudoku_size < 0)
    {
        if (ENABLE_DEBUG_MODE)
            fprintf(
                stderr,
                "[ERR msg] Received NULL or negative Sudoku grid or Sudoku size %d! (Sudoku size must be non-negative)\n",
                sudoku_size);
        exit(EXIT_FAILURE);
    }

    HeaderNode* result_ptr = dlx_initialize_sudoku_matrix_compact(sudoku_size);

    int sudoku_cells = (int)pow(sudoku_size, 2);
    for (int grid_index = 0; grid_index < sudoku_cells; grid_index++)
    {
        int sudoku_row = (grid_index / sudoku_size) + 1;
        int sudoku_column = (grid_index % sudoku_size) + 1;
        int sudoku_value = sudoku_grid[grid_index];

        if (sudoku_value > 0)
        {
            dlx_set_constraint(result_ptr, sudoku_grid, sudoku_size, sudoku_row, sudoku_column,
                               sudoku_value);
        }
    }

    return result_ptr;
}

bool dlx_node_covered(Node* node_ptr)
{
    return !(node_ptr->up->down == node_ptr && node_ptr->down->up == node_ptr);
}

/**
 * @brief Uncovers a specific node in the Dancing Links (DLX) matrix.
 *
 * This function performs modifications on the DLX matrix where a previously covered node is restored
 * to the vertical circular doubly linked list of column nodes, allowing its visibility for future operations.
 * Additionally, the `current_size` attribute of the associated header column is incremented
 * in its parent header node to reflect the covering of the column header node.
 *
 * @param node_ptr Pointer to the node to be uncovered. This node must be part of a valid DLX structure
 *                 and linked to a valid header node. The node's `up` and `down` pointers must reference
 *                 other valid nodes within the same column or the column header itself.
 */
void dlx_uncover_node(Node* node_ptr)
{
    dlx_link_nodes_vertical(node_ptr->up, node_ptr);
    dlx_link_nodes_vertical(node_ptr, node_ptr->down);
    node_ptr->header_node->current_size++;
}

/**
 * @brief Covers a specific node in the Dancing Links (DLX) matrix.
 *
 * This function performs modifications on the DLX matrix where a previously uncovered node is hidden
 * from the vertical circular doubly linked list of column nodes, removing its visibility for future operations.
 * Additionally, the `current_size` attribute of the associated header column is incremented
 * in its parent header node to reflect the covering of the column header node.
 *
 * @param node_ptr Pointer to the node to be covered. This node must be part of a valid DLX structure
 *                 and linked to a valid header node. The node's `up` and `down` pointers must reference
 *                 other valid nodes within the same column or the column header itself.
 */
void dlx_cover_node(Node* node_ptr)
{
    dlx_link_nodes_vertical(node_ptr->up, node_ptr->down);
    node_ptr->header_node->current_size--;
}

// TODO: Update description.
/**
 * @brief Uncovers a specific column header node in the Dancing Links (DLX) matrix.
 *
 * This function performs modifications on the DLX matrix where a previously covered column header node is restored
 * to the horizontal circular doubly linked list of column headers, allowing its visibility for future operations.
 *
 * @param column_ptr Pointer to the specific column header node that is being uncovered and restored within the
 *                   horizontal list of column headers.
 */
void dlx_uncover_column_header(HeaderNode* header_ptr, HeaderNode* column_ptr)
{
    dlx_link_nodes_horizontal(column_ptr->node.left, &(column_ptr->node));
    dlx_link_nodes_horizontal(&(column_ptr->node), column_ptr->node.right);
    header_ptr->current_size++;
}

// TODO: Update description.
/**
 * @brief Covers a specific column header node in the Dancing Links (DLX) matrix.
 *
 * This function performs modifications on the DLX matrix where a previously uncovered column header node is hidden
 * from the horizontal circular doubly linked list of column headers, removing its visibility for future operations.
 *
 * @param column_ptr Pointer to the specific column header node that is being covered and hidden within the
 *                   horizontal list of column headers.
 */
void dlx_cover_column_header(HeaderNode* header_ptr, HeaderNode* column_ptr)
{
    dlx_link_nodes_horizontal(column_ptr->node.left, column_ptr->node.right);
    header_ptr->current_size--;
}

// TODO: Update description.
/**
 * @brief Uncovers all nodes affected within specified constraint columns in a Dancing Links (DLX) matrix.
 *
 * This function restores columns and nodes that were previously covered, ensuring that all affected
 * nodes and column headers are linked back into the DLX matrix. It iterates over targeted constraint columns and
 * for each column, traverses all related rows and nodes, uncovering them as required while maintaining consistency.
 *
 * @param constraint_column_ptrs Array of pointers to header nodes representing constraint columns to be uncovered.
 *                               Each pointer specifies a column impacted by prior operations.
 * @param constraint_column_ptrs_length The number of constraint columns contained in the `constraint_column_ptrs` array.
 *                                       Indicates how many columns are to be processed.
 *
 * @note Each constraint column in the array is fully uncovered. This involves restoring the column header to its
 *       previous position in the horizontal circular linked list and uncovering all associated nodes within rows of that column.
 * @note If a node does not belong to any of the provided constraint columns, it is specifically uncovered to restore consistency.
 * @note This function internally uses the `dlx_uncover_column_header` and `dlx_uncover_node` operations for managing
 *       column and node uncovering processes.
 */
void dlx_uncover_affected_nodes(HeaderNode* header_ptr, HeaderNode** constraint_column_ptrs, int constraint_column_ptrs_length)
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
                bool safe_to_uncover = true;

                // Check if the neighboring node is in any of the columns being covered or is already uncovered.
                for (int check_index = 0; check_index < constraint_column_ptrs_length; check_index++)
                {
                    if (row_node_ptr->header_node == constraint_column_ptrs[check_index] || !dlx_node_covered(row_node_ptr))
                    {
                        safe_to_uncover = false;
                        break;
                    }
                }

                // If the neighboring node is not in any of the columns being covered, cover it.
                if (safe_to_uncover) dlx_uncover_node(row_node_ptr);
            }
        }
    }
}

// TODO: Update description.
/**
 * @brief Covers all nodes affected within specified constraint columns in a Dancing Links (DLX) matrix.
 *
 * This function hides columns and nodes that were previously uncovered, ensuring that all affected
 * nodes and column headers are "invisible" when traversing the DLX matrix. It iterates over targeted constraint columns and
 * for each column, traverses all related rows and nodes, covering them as required while maintaining consistency.
 *
 * @param constraint_column_ptrs Array of pointers to header nodes representing constraint columns to be covered.
 *                               Each pointer specifies a column impacted by prior operations.
 * @param constraint_column_ptrs_length The number of constraint columns contained in the `constraint_column_ptrs` array.
 *                                       Indicates how many columns are to be processed.
 *
 * @note Each constraint column in the array is fully covered. This involves hiding the column header from its
 *       previous position in the horizontal circular linked list and covering all associated nodes within rows of that column.
 * @note If a node does not belong to any of the provided constraint columns, it is specifically covered to restore consistency.
 * @note This function internally uses the `dlx_cover_column_header` and `dlx_cover_node` operations for managing
 *       column and node uncovering processes.
 */
void dlx_cover_affected_nodes(HeaderNode* header_ptr, HeaderNode** constraint_column_ptrs, int constraint_column_ptrs_length)
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
                bool safe_to_cover = true;

                // Check if the neighboring node is in any of the columns being covered or is already covered.
                for (int check_index = 0; check_index < constraint_column_ptrs_length; check_index++)
                {
                    if (row_node_ptr->header_node == constraint_column_ptrs[check_index] || dlx_node_covered(row_node_ptr))
                    {
                        safe_to_cover = false;
                        break;
                    }
                }

                // If the neighboring node is not in any of the columns being covered, cover it.
                if (safe_to_cover) dlx_cover_node(row_node_ptr);
            }
        }
    }
}

// TODO: Update description.
/**
 * @brief Includes a specific node in the DLX matrix and covers its related constraints.
 *
 * This function identifies the constraint columns related to a given node in a Sudoku-specific DLX matrix.
 * It computes the associated constraint columns for cell, row, column, and block constraints based on
 * the node's values (row, column, and value) and the Sudoku grid size. These computed columns are then
 * used to cover the necessary nodes involved in satisfying these constraints.
 *
 * @param header_ptr Pointer to the starting header node of the DLX matrix,
 *                   representing the starting point for column traversal.
 *                   This header node helps locate the constraint columns required for the operation.
 * @param node_ptr Pointer to the node in the DLX matrix that represents a specific value assignment
 *                 to a cell in the Sudoku grid. The node's properties (sudoku_row, sudoku_column,
 *                 sudoku_value) are used for calculating the associated constraints.
 *                 This node is deemed as a part of the solution of the given Sudoku puzzle.
 * @param sudoku_size The size of the Sudoku grid (e.g., 9 for a 9x9 grid). Assumes it is a perfect square,
 *                    as this is required to compute block constraints correctly.
 *
 * @note The function dynamically allocates memory for the array of constraint column pointers, which is
 *       released before exiting the function.
 * @note This operation is essential for temporarily "including" a value in the grid representation
 *       for solving or validation purposes.
 */
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

// TODO: Update description.
/**
 * @brief Excludes a node and its associated constraints from the Dancing Links (DLX) matrix.
 *
 * This function identifies and collects column headers associated with the given node by moving through
 * its rightward links. These column headers correspond to constraints connected to the given node.
 * The function then invokes `dlx_uncover_affected_nodes` to mark these constraints as uncovered, effectively
 * excluding the node and its constraints from further consideration during the DLX algorithm's execution.
 *
 * @param node_ptr Pointer to the node in the DLX matrix to exclude. This node represents a specific solution
 *                 element, and its exclusion is achieved by handling its associated constraints.
 */
void dlx_exclude_node(HeaderNode* header_ptr, Node* node_ptr)
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

/**
 * @brief Sets a constraint in the Dancing Links (DLX) matrix for a given Sudoku grid cell and value.
 *
 * This function enforces a constraint in a DLX matrix by linking a node to the matrix corresponding
 * to a particular Sudoku grid cell and value. It ensures that the value is valid and finds the appropriate
 * column node in the matrix. If successful, it updates the Sudoku grid and returns the node
 * representing the constraint.
 *
 * @param header_ptr Pointer to the header node of the DLX matrix, used as a starting point for column traversal.
 *                   The header node links all column nodes in the matrix.
 * @param sudoku_grid Pointer to a 1D array representing the Sudoku board. This is updated to reflect
 *                    the constraint if it is successfully set.
 * @param sudoku_size The size of the Sudoku grid. Typically, 9 for a standard Sudoku puzzle.
 * @param sudoku_row The row number of the Sudoku grid cell for which the constraint needs to be set.
 *                   Must be between 1 and sudoku_size (inclusive).
 * @param sudoku_column The column number of the Sudoku grid cell for which the constraint needs to be set.
 *                      Must be between 1 and sudoku_size (inclusive).
 * @param sudoku_value The value to place in the specified Sudoku grid cell. Must be between 1 and sudoku_size (inclusive).
 *
 * @return Pointer to the node in the DLX matrix representing the successfully set constraint, or NULL
 *         if the constraint could not be set due to invalid parameters or the target node not existing.
 *
 * @note Exits with a printed error message if the specified row, column, or value is out of bounds,
 *       or if a column or node corresponding to the constraint is not found.
 */
Node* dlx_set_constraint(HeaderNode* header_ptr, int* sudoku_grid, int sudoku_size, int sudoku_row, int sudoku_column, int sudoku_value)
{
    if (sudoku_row < 1 || sudoku_row > sudoku_size ||
        sudoku_column < 1 || sudoku_column > sudoku_size ||
        sudoku_value < 1 || sudoku_value > sudoku_size)
    {
        if (ENABLE_DEBUG_MODE)
            fprintf(stderr, "[ERR msg] Constraint r%dc%d#%d not set! (value out of bounds)\n", sudoku_row,
                    sudoku_column,
                    sudoku_value);
        return NULL;
    }
    int grid_index = (sudoku_row - 1) * sudoku_size + sudoku_column - 1;

    if (header_ptr == NULL)
    {
        sudoku_grid[grid_index] = sudoku_value;
        if (ENABLE_DEBUG_MODE)
            printf("[SCS msg] Constraint set without associated header pointer! (r%dc%d#%d)\n", sudoku_row, sudoku_column,
                   sudoku_value);
        return NULL;
    }

    int matrix_column_index = (sudoku_row - 1) * sudoku_size + sudoku_column;
    HeaderNode* column_ptr = dlx_get_column(header_ptr, matrix_column_index);
    if (column_ptr == NULL)
    {
        if (ENABLE_DEBUG_MODE)
            if (ENABLE_DEBUG_MODE)
                fprintf(stderr, "[ERR msg] Constraint r%dc%d#%d not set! (column not found)\n", sudoku_row,
                        sudoku_column,
                        sudoku_value);
        return NULL;
    }

    for (Node* node_ptr = column_ptr->node.down; node_ptr != &(column_ptr->node); node_ptr = node_ptr->down)
    {
        if (node_ptr->sudoku_value == sudoku_value)
        {
            dlx_include_node(header_ptr, node_ptr, sudoku_size);
            sudoku_grid[grid_index] = sudoku_value;

            if (ENABLE_DEBUG_MODE)
                printf("[SCS msg] Constraint set with associated header pointer! (r%dc%d#%d)\n", sudoku_row, sudoku_column,
                       sudoku_value);
            return node_ptr;
        }
    }

    if (ENABLE_DEBUG_MODE)
        fprintf(stderr, "[ERR msg] Constraint r%dc%d#%d not set! (node not found in column)\n", sudoku_row,
                sudoku_column,
                sudoku_value);
    return NULL;
}

// TODO: Add description.
void dlx_remove_constraint(HeaderNode* header_ptr, Node* node_ptr, int* sudoku_grid, int sudoku_size)
{
    dlx_exclude_node(header_ptr, node_ptr);

    int sudoku_row = node_ptr->sudoku_row;
    int sudoku_column = node_ptr->sudoku_column;
    int sudoku_value = node_ptr->sudoku_value;
    int grid_index = (sudoku_row - 1) * sudoku_size + sudoku_column - 1;
    sudoku_grid[grid_index] = 0;

    if (ENABLE_DEBUG_MODE)
        printf("[SCS msg] Constraint removed! (r%dc%d#%d)\n", sudoku_row, sudoku_column,
               sudoku_value);
}

// TODO: Add description.
int dlx_sudoku_solution_quantity_quick(HeaderNode* header_ptr, int sudoku_size)
{
    HeaderNode* selected_column_ptr = dlx_get_column_shortest(header_ptr);
    if (header_ptr->current_size == 0 || selected_column_ptr == NULL)
    {
        if (ENABLE_DEBUG_MODE)printf("[SCS msg] Unique solution found!\n");
        return 1;
    }

    if (selected_column_ptr->current_size < 1) return 0;

    // Uncomment for debugging purposes:
    // dlx_print_matrix_full(header_ptr);

    int result = 0;
    for (Node* selected_node_ptr = selected_column_ptr->node.down;
         selected_node_ptr != &(selected_column_ptr->node);
         selected_node_ptr = selected_node_ptr->down)
    {
        // Uncomment for debugging purposes:
        // printf("[PCKD: C%3d ]", selected_column_ptr->num);
        // dlx_print_node(selected_node_ptr);
        // printf("\n\n");

        dlx_include_node(header_ptr, selected_node_ptr, sudoku_size);
        result += dlx_sudoku_solution_quantity_quick(header_ptr, sudoku_size);
        if (result > 1) return 2;

        dlx_exclude_node(header_ptr, selected_node_ptr);
    }

    return result;
}

// TODO: Update description.
/**
 * @brief Solves a Sudoku puzzle encoded in a Dancing Links (DLX) matrix using the exact cover algorithm.
 *
 * This function implements the recursive backtracking algorithm to solve a Sudoku puzzle.
 * It selects the column with the fewest uncovered nodes, iterates through the rows in
 * that column, includes the row in the solution, and recursively attempts to solve it. If
 * no solution is found, it backtracks by excluding the previously selected row.
 *
 * @param header_ptr Pointer to the starting header node of the DLX matrix. Represents the
 *                   entry point for matrix traversal and operations.
 * @param sudoku_grid Pointer to an integer array representing the Sudoku grid. The solution
 *                    will be written into this array if the puzzle is successfully solved.
 *                    The array is expected to be of the size sudoku_size * sudoku_size.
 * @param sudoku_size The size of the Sudoku grid (e.g., 9 for a standard 9x9 Sudoku puzzle).
 *                    Assumes the puzzle is of dimension sudoku_size x sudoku_size.
 *
 * @return A boolean value indicating whether a solution was found.
 *         Returns true if the puzzle is successfully solved, otherwise false if no solution exists.
 *
 * @note The provided Sudoku grid is expected to be already encoded into the DLX matrix using
 *       constraints before calling this function.
 * @note If the matrix is empty or a column with no valid rows is encountered, the function
 *       backtracks or determines the solution status accordingly.
 */
bool dlx_sudoku_solve_inner(HeaderNode* header_ptr, int* sudoku_grid, int sudoku_size)
{
    HeaderNode* selected_column_ptr = dlx_get_column_shortest(header_ptr);
    if (header_ptr->current_size == 0 || selected_column_ptr == NULL)
    {
        if (ENABLE_DEBUG_MODE)printf("[SCS msg] Solution found!\n");
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
        if (dlx_sudoku_solve_inner(header_ptr, sudoku_grid, sudoku_size))
        {
            int selected_node_row_index = selected_node_ptr->sudoku_row - 1;
            int selected_node_column_index = selected_node_ptr->sudoku_column - 1;
            int selected_node_value = selected_node_ptr->sudoku_value;

            int digits_index = selected_node_row_index * sudoku_size + selected_node_column_index;
            sudoku_grid[digits_index] = selected_node_value;

            return true;
        }
        dlx_exclude_node(header_ptr, selected_node_ptr);
    }

    return false;
}

int* dlx_sudoku_solve(int* sudoku_grid_unsolved, int sudoku_size)
{
    if (ENABLE_DEBUG_MODE) printf("\n\n[SCS msg] Solving Sudoku puzzle...\n");
    int sudoku_cells = (int)pow(sudoku_size, 2);
    int* sudoku_grid_solved = (int*)malloc(sizeof(int) * sudoku_size * sudoku_size);
    memcpy(sudoku_grid_solved, sudoku_grid_unsolved, sizeof(int) * sudoku_cells);

    HeaderNode* header_ptr = dlx_initialize_sudoku_matrix_from_grid(sudoku_grid_solved, sudoku_size);
    if (dlx_sudoku_solve_inner(header_ptr, sudoku_grid_solved, sudoku_size))
        return sudoku_grid_solved;
    else
    {
        free(sudoku_grid_solved);
        return NULL;
    }
}

// TODO: Add description.
bool dlx_sudoku_generate_unique_unsolved_inner(int* sudoku_grid, int sudoku_size)
{
    HeaderNode* copy_header_ptr = dlx_initialize_sudoku_matrix_from_grid(sudoku_grid, sudoku_size);
    int solution_quantity = dlx_sudoku_solution_quantity_quick(copy_header_ptr, sudoku_size);

    switch (solution_quantity)
    {
    case 0:
        dlx_terminate_matrix(copy_header_ptr);
        return false;
    case 1:
        dlx_terminate_matrix(copy_header_ptr);
        if (ENABLE_DEBUG_MODE)printf("[SCS msg] Unique puzzle generated!\n");
        return true;
    case 2:
    default:

        int sudoku_cells = (int)pow(sudoku_size, 2);
        HeaderNode* selected_column_ptr = NULL;

        while (selected_column_ptr == NULL)
        {
            int selected_matrix_column_index = random_interval(1, sudoku_cells);
            selected_column_ptr = dlx_get_column(copy_header_ptr, selected_matrix_column_index);
        }

        int selected_matrix_row_index = random_interval(1, selected_column_ptr->current_size);
        Node* selected_node_ptr = selected_column_ptr->node.down;
        for (int ix = 1; ix < selected_matrix_row_index; ix++)
            selected_node_ptr = selected_node_ptr->down;

        int sudoku_row = selected_node_ptr->sudoku_row;
        int sudoku_column = selected_node_ptr->sudoku_column;
        int sudoku_value = selected_node_ptr->sudoku_value;
        dlx_set_constraint(copy_header_ptr, sudoku_grid, sudoku_size, sudoku_row, sudoku_column, sudoku_value);
        if (ENABLE_DEBUG_MODE) printf("[SCS msg] Set constraint: r%dc%d#%d\n", sudoku_row, sudoku_column, sudoku_value);

        dlx_terminate_matrix(copy_header_ptr);
        return dlx_sudoku_generate_unique_unsolved_inner(sudoku_grid, sudoku_size);
    }
}

// TODO: Add description.
int* dlx_sudoku_generate_unique_unsolved(int sudoku_size, enum DIFFICULTY additional_constraints)
{
    if (ENABLE_DEBUG_MODE) printf("\n\n[SCS msg] Generating unique unsolved Sudoku puzzle...\n");
    int sudoku_cells = (int)pow(sudoku_size, 2);
    int* sudoku_grid_unsolved = (int*)calloc(sudoku_cells, sizeof(int));
    while (dlx_sudoku_generate_unique_unsolved_inner(sudoku_grid_unsolved, sudoku_size) == false)
    {
        if (ENABLE_DEBUG_MODE) printf("[SCS msg] Puzzle generation failed! Retrying...\n");
        memset(sudoku_grid_unsolved, 0, sudoku_cells * sizeof(int));
    };
    int* sudoku_grid_solved = dlx_sudoku_solve(sudoku_grid_unsolved, sudoku_size);

    if (ENABLE_DEBUG_MODE) printf("[INF msg] Setting additional hints.\n");
    for (int constraint_num = 1; constraint_num <= additional_constraints; constraint_num++)
    {
        int sudoku_row;
        int sudoku_column;
        int grid_index;
        do
        {
            sudoku_row = random_interval(1, sudoku_size);
            sudoku_column = random_interval(1, sudoku_size);
            grid_index = (sudoku_row - 1) * sudoku_size + sudoku_column - 1;
        }
        while (!(sudoku_grid_unsolved[grid_index] == 0 && sudoku_grid_solved[grid_index] > 0));

        int sudoku_value = sudoku_grid_solved[grid_index];
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, sudoku_row, sudoku_column, sudoku_value);
        if (ENABLE_DEBUG_MODE) printf("[SCS msg] Set additional constraint #%d: r%dc%d#%d\n", constraint_num, sudoku_row, sudoku_column, sudoku_value);
    }

    free(sudoku_grid_solved);
    return sudoku_grid_unsolved;
}

int grid_count_filled_cells(int* sudoku_grid, int sudoku_size)
{
    int count = 0;
    for (int ix = 0; ix < sudoku_size * sudoku_size; ix++)
    {
        if (sudoku_grid[ix] > 0)
            count++;
    }
    return count;
}

SudokuGrid* dlx_sudoku_generate_unique_pair(int sudoku_size, enum DIFFICULTY additional_constraints)
{
    SudokuGrid* sudoku = grid_create(sudoku_size);
    sudoku->sudoku_unsolved = dlx_sudoku_generate_unique_unsolved(sudoku_size, additional_constraints);
    sudoku->sudoku_solved = dlx_sudoku_solve(sudoku->sudoku_unsolved, sudoku_size);
    return sudoku;
}

#ifndef MAIN_ACTIVE
int main()
{
    // Change the parameters of the executed test here:
    int sudoku_size = 9;
    enum DIFFICULTY difficulty = EXTREME;
    int test_case_index = 5;

    // Changing the code below this comment will alter the execution of the algorithm. It is advised to have a brief understanding of the functions used before proceeding.
    int sudoku_cells = (int)pow(sudoku_size, 2);
    int* sudoku_grid_unsolved = (int*)calloc(sudoku_cells, sizeof(int));
    int* sudoku_grid_solved;

    switch (test_case_index)
    {
    default:
    case 0:
        // Solve an empty Sudoku puzzle.
        sudoku_grid_solved = dlx_sudoku_solve(sudoku_grid_unsolved, sudoku_size);
        break;
    case 1:
        // Diagonal constraint.
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 1, 1, 1);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 2, 2, 2);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 3, 3, 3);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 4, 4, 4);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 5, 5, 5);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 6, 6, 6);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 7, 7, 7);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 8, 8, 8);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 9, 9, 9);
        break;
    case 2:
        // Block constraint at (0,0).
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 1, 1, 1);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 1, 2, 2);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 1, 3, 3);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 2, 1, 4);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 2, 2, 5);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 2, 3, 6);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 3, 1, 7);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 3, 2, 8);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 3, 3, 9);
        break;
    case 3:
        // Block constraint at (7,7).
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 7, 7, 1);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 7, 8, 2);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 7, 9, 3);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 8, 7, 4);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 8, 8, 5);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 8, 9, 6);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 9, 7, 7);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 9, 8, 8);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 9, 9, 9);
        break;
    case 4:
        // Constraints in the middle of blocks.
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 2, 2, 1);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 2, 5, 2);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 2, 8, 3);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 5, 2, 4);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 5, 5, 5);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 5, 8, 6);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 8, 2, 7);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 8, 5, 8);
        dlx_set_constraint(NULL, sudoku_grid_unsolved, sudoku_size, 8, 8, 9);
        break;
    case 5:
        // Randomly generated Sudoku puzzle with a unique solution.
        free(sudoku_grid_unsolved);
        sudoku_grid_unsolved = dlx_sudoku_generate_unique_unsolved(sudoku_size, difficulty);
        break;
    }

    sudoku_grid_solved = dlx_sudoku_solve(sudoku_grid_unsolved, sudoku_size);
    free(sudoku_grid_unsolved);
    free(sudoku_grid_solved);
    return 0;
}
#endif
