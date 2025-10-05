#include <gtk/gtk.h>
#include <time.h>
#include <stdlib.h>
#include "sudoku_algorithm.h"
#include "sudoku_main.h"

#define GRID_SIZE 9
#define GRID_BLOCK_SIZE 3
#define OUTER_MARGIN 20
#define CELL_SIZE 50

static SudokuGrid* current_puzzle = NULL;
static enum DIFFICULTY current_difficulty = EASY;

static void calculate_new_position(guint keyval, int current_col, int current_row, int* new_col, int* new_row)
{
    *new_col = current_col;
    *new_row = current_row;

    switch (keyval)
    {
    case GDK_KEY_Up:
        *new_row = (current_row > 0) ? current_row - 1 : current_row;
        break;
    case GDK_KEY_Down:
        *new_row = (current_row < GRID_SIZE - 1) ? current_row + 1 : current_row;
        break;
    case GDK_KEY_Left:
        *new_col = (current_col > 0) ? current_col - 1 : current_col;
        break;
    case GDK_KEY_Right:
        *new_col = (current_col < GRID_SIZE - 1) ? current_col + 1 : current_col;
        break;
    default:
        break;
    }
}

static gboolean on_key_pressed(GtkEventControllerKey* controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data)
{
    GtkWidget* entry = GTK_WIDGET(user_data);
    GtkWidget* grid = gtk_widget_get_parent(entry);

    int current_col, current_row;
    gtk_grid_query_child(GTK_GRID(grid), entry, &current_col, &current_row, NULL, NULL);

    int new_col, new_row;
    calculate_new_position(keyval, current_col, current_row, &new_col, &new_row);

    if (new_row != current_row || new_col != current_col)
    {
        GtkWidget* target_entry = gtk_grid_get_child_at(GTK_GRID(grid), new_col, new_row);
        if (target_entry)
        {
            gtk_widget_grab_focus(target_entry);
            return TRUE;
        }
    }

    return FALSE;
}

static gboolean reset_cell_style(gpointer user_data)
{
    GtkWidget* entry = GTK_WIDGET(user_data);
    GtkStyleContext* sc = gtk_widget_get_style_context(entry);

    gtk_style_context_remove_class(sc, "correct-value");
    gtk_style_context_remove_class(sc, "incorrect-value");

    return G_SOURCE_REMOVE;
}

static gboolean clear_entry_text(gpointer user_data)
{
    GtkEditable* editable = GTK_EDITABLE(user_data);
    gtk_editable_set_text(editable, "");
    return G_SOURCE_REMOVE;
}

static void on_entry_changed(GtkEditable* editable, gpointer user_data)
{
    GtkWidget* entry = GTK_WIDGET(editable);
    GtkWidget* grid = gtk_widget_get_parent(entry);
    const char* text = gtk_editable_get_text(editable);

    if (!current_puzzle || !text || text[0] == '\0')
        return;

    int col, row;
    gtk_grid_query_child(GTK_GRID(grid), entry, &col, &row, NULL, NULL);
    int user_value = text[0] - '0';
    int grid_index = row * GRID_SIZE + col;
    int correct_value = current_puzzle->sudoku_solved[grid_index];

    GtkStyleContext* sc = gtk_widget_get_style_context(entry);

    if (user_value >= 1 && user_value <= 9)
    {
        if (user_value == correct_value)
        {
            // Correct value - briefly show green border, then make non-editable
            gtk_style_context_add_class(sc, "correct-value");
            gtk_style_context_add_class(sc, "user-filled");
            g_timeout_add(300, reset_cell_style, entry);

            // Make the cell non-editable after correct entry
            gtk_editable_set_editable(editable, FALSE);
            gtk_widget_set_can_focus(entry, FALSE);
        }
        else
        {
            // Incorrect value - briefly show red border and clear the cell
            gtk_style_context_add_class(sc, "incorrect-value");
            g_timeout_add(300, reset_cell_style, entry);
            g_idle_add(clear_entry_text, editable);
        }
    }
    else
    {
        // Invalid input - clear the cell
        g_idle_add(clear_entry_text, editable);
    }
}

static void load_css_styling(void)
{
    GtkCssProvider* provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, "gui.css");
    GdkDisplay* display = gdk_display_get_default();
    gtk_style_context_add_provider_for_display(display,GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_object_unref(provider);
}

static GtkWidget* create_cell_entry(int row_index, int column_index)
{
    GtkWidget* entry = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry), 1);
    gtk_entry_set_alignment(GTK_ENTRY(entry), 0.5);
    gtk_editable_set_position(GTK_EDITABLE(entry), 0);
    gtk_widget_set_size_request(entry, CELL_SIZE, CELL_SIZE);

    GtkStyleContext* sc = gtk_widget_get_style_context(entry);
    gtk_style_context_add_class(sc, "cell");

    if ((column_index + 1) % GRID_BLOCK_SIZE == 0 && column_index != GRID_SIZE - 1)
        gtk_style_context_add_class(sc, "thick-right");
    if ((row_index + 1) % GRID_BLOCK_SIZE == 0 && row_index != GRID_SIZE - 1)
        gtk_style_context_add_class(sc, "thick-bottom");

    GtkEventController* key_controller = gtk_event_controller_key_new();
    g_signal_connect(key_controller, "key-pressed", G_CALLBACK(on_key_pressed), entry);
    gtk_widget_add_controller(entry, key_controller);

    g_signal_connect(entry, "changed", G_CALLBACK(on_entry_changed), NULL);

    return entry;
}

static GtkWidget* create_sudoku_grid(void)
{
    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 0);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 0);
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);

    gtk_widget_set_margin_top(grid, OUTER_MARGIN);
    gtk_widget_set_margin_bottom(grid, OUTER_MARGIN);
    gtk_widget_set_margin_start(grid, OUTER_MARGIN);
    gtk_widget_set_margin_end(grid, OUTER_MARGIN);

    gtk_widget_set_hexpand(grid, TRUE);
    gtk_widget_set_vexpand(grid, TRUE);

    GtkStyleContext* grid_sc = gtk_widget_get_style_context(grid);
    gtk_style_context_add_class(grid_sc, "sudoku-grid");

    for (int row_index = 0; row_index < GRID_SIZE; row_index++)
    {
        for (int column_index = 0; column_index < GRID_SIZE; column_index++)
        {
            GtkWidget* entry = create_cell_entry(row_index, column_index);
            gtk_grid_attach(GTK_GRID(grid), entry, column_index, row_index, 1, 1);
        }
    }

    return grid;
}

static void on_generate_board_clicked(GtkButton* button, gpointer user_data)
{
    GtkWidget* grid = GTK_WIDGET(user_data);

    // Free previous puzzle if exists
    if (current_puzzle)
    {
        free(current_puzzle->sudoku_unsolved);
        free(current_puzzle->sudoku_solved);
        free(current_puzzle);
    }

    // Generate new puzzle
    current_puzzle = dlx_sudoku_generate_unique_pair(GRID_SIZE, current_difficulty);

    // Populate the grid with the unsolved puzzle
    for (int row_index = 0; row_index < GRID_SIZE; row_index++)
    {
        for (int column_index = 0; column_index < GRID_SIZE; column_index++)
        {
            GtkWidget* entry = gtk_grid_get_child_at(GTK_GRID(grid), column_index, row_index);
            if (entry && GTK_IS_ENTRY(entry))
            {
                int grid_index = row_index * GRID_SIZE + column_index;
                int value = current_puzzle->sudoku_unsolved[grid_index];

                GtkStyleContext* sc = gtk_widget_get_style_context(entry);

                if (value > 0)
                {
                    char text[2];
                    text[0] = '0' + value;
                    text[1] = '\0';
                    gtk_editable_set_text(GTK_EDITABLE(entry), text);
                    gtk_editable_set_editable(GTK_EDITABLE(entry), FALSE);
                    gtk_widget_set_can_focus(entry, FALSE);

                    gtk_style_context_add_class(sc, "prefilled");
                    gtk_style_context_remove_class(sc, "user-filled");
                }
                else
                {
                    gtk_editable_set_text(GTK_EDITABLE(entry), "");
                    gtk_editable_set_editable(GTK_EDITABLE(entry), TRUE);
                    gtk_widget_set_can_focus(entry, TRUE);

                    gtk_style_context_remove_class(sc, "prefilled");
                    gtk_style_context_remove_class(sc, "user-filled");
                }
            }
        }
    }
}

static void on_clear_board_clicked(GtkButton* button, gpointer user_data)
{
    GtkWidget* grid = GTK_WIDGET(user_data);

    for (int row_index = 0; row_index < GRID_SIZE; row_index++)
    {
        for (int column_index = 0; column_index < GRID_SIZE; column_index++)
        {
            GtkWidget* entry = gtk_grid_get_child_at(GTK_GRID(grid), column_index, row_index);
            if (entry && GTK_IS_ENTRY(entry))
            {
                gtk_editable_set_text(GTK_EDITABLE(entry), "");
                gtk_editable_set_editable(GTK_EDITABLE(entry), TRUE);
                gtk_widget_set_can_focus(entry, TRUE);

                GtkStyleContext* sc = gtk_widget_get_style_context(entry);
                gtk_style_context_remove_class(sc, "prefilled");
                gtk_style_context_remove_class(sc, "user-filled");
            }
        }
    }

    // Free the current puzzle
    if (current_puzzle)
    {
        free(current_puzzle->sudoku_unsolved);
        free(current_puzzle->sudoku_solved);
        free(current_puzzle);
        current_puzzle = NULL;
    }
}

static void on_solve_board_clicked(GtkButton* button, gpointer user_data)
{
    GtkWidget* grid = GTK_WIDGET(user_data);

    // Collect the current state of the board
    int* current_board = (int*)calloc(GRID_SIZE * GRID_SIZE, sizeof(int));

    for (int row_index = 0; row_index < GRID_SIZE; row_index++)
    {
        for (int column_index = 0; column_index < GRID_SIZE; column_index++)
        {
            GtkWidget* entry = gtk_grid_get_child_at(GTK_GRID(grid), column_index, row_index);
            if (entry && GTK_IS_ENTRY(entry))
            {
                const char* current_text = gtk_editable_get_text(GTK_EDITABLE(entry));
                int grid_index = row_index * GRID_SIZE + column_index;

                if (current_text && current_text[0] >= '1' && current_text[0] <= '9')
                {
                    current_board[grid_index] = current_text[0] - '0';
                }
                else
                {
                    current_board[grid_index] = 0;
                }
            }
        }
    }

    // Solve the current board state
    int* solved_board = dlx_sudoku_solve(current_board, GRID_SIZE);

    if (!solved_board)
    {
        // If solving fails, free memory and return
        free(current_board);
        // Optionally show an error message to user
        return;
    }

    // Update current_puzzle with the solved board
    if (current_puzzle)
    {
        free(current_puzzle->sudoku_unsolved);
        free(current_puzzle->sudoku_solved);
        free(current_puzzle);
    }

    current_puzzle = (SudokuGrid*)malloc(sizeof(SudokuGrid));
    current_puzzle->sudoku_unsolved = current_board;
    current_puzzle->sudoku_solved = solved_board;

    // Fill in all remaining empty cells with the solved values
    for (int row_index = 0; row_index < GRID_SIZE; row_index++)
    {
        for (int column_index = 0; column_index < GRID_SIZE; column_index++)
        {
            GtkWidget* entry = gtk_grid_get_child_at(GTK_GRID(grid), column_index, row_index);
            if (entry && GTK_IS_ENTRY(entry))
            {
                const char* current_text = gtk_editable_get_text(GTK_EDITABLE(entry));
                int grid_index = row_index * GRID_SIZE + column_index;

                // Only fill cells that are currently empty
                if (!current_text || current_text[0] == '\0')
                {
                    int solved_value = solved_board[grid_index];

                    if (solved_value > 0)
                    {
                        char text[2];
                        text[0] = '0' + solved_value;
                        text[1] = '\0';
                        gtk_editable_set_text(GTK_EDITABLE(entry), text);
                        gtk_editable_set_editable(GTK_EDITABLE(entry), FALSE);
                        gtk_widget_set_can_focus(entry, FALSE);

                        GtkStyleContext* sc = gtk_widget_get_style_context(entry);
                        gtk_style_context_add_class(sc, "user-filled");
                    }
                }
            }
        }
    }
}

static void on_difficulty_changed(GtkDropDown* dropdown, GParamSpec* pspec, gpointer user_data)
{
    guint selected = gtk_drop_down_get_selected(dropdown);

    switch (selected)
    {
    case 0:
        current_difficulty = EASY;
        break;
    case 1:
        current_difficulty = MEDIUM;
        break;
    case 2:
        current_difficulty = HARD;
        break;
    case 3:
        current_difficulty = EXTREME;
        break;
    default:
        current_difficulty = EASY;
        break;
    }
}

static void on_settings_clicked(GtkButton* button, gpointer user_data)
{
    GtkWidget* parent_window = GTK_WIDGET(user_data);

    // Create settings dialog
    GtkWidget* dialog = gtk_dialog_new_with_buttons(
        "Settings",
        GTK_WINDOW(parent_window),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "_Close",
        GTK_RESPONSE_CLOSE,
        NULL
    );

    gtk_window_set_default_size(GTK_WINDOW(dialog), 300, 150);

    // Create content box
    GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget* content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_top(content_box, 20);
    gtk_widget_set_margin_bottom(content_box, 20);
    gtk_widget_set_margin_start(content_box, 20);
    gtk_widget_set_margin_end(content_box, 20);

    // Create label
    GtkWidget* label = gtk_label_new("Choose the dfficulty of the next puzzle:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);

    // Create dropdown with difficulty options
    const char* difficulties[] = {"Easy", "Medium", "Hard", "Extreme", NULL};
    GtkWidget* dropdown = gtk_drop_down_new_from_strings(difficulties);

    // Set current selection based on current_difficulty
    guint current_selection = 0;
    switch (current_difficulty)
    {
    case EASY:
        current_selection = 0;
        break;
    case MEDIUM:
        current_selection = 1;
        break;
    case HARD:
        current_selection = 2;
        break;
    case EXTREME:
        current_selection = 3;
        break;
    }
    gtk_drop_down_set_selected(GTK_DROP_DOWN(dropdown), current_selection);

    // Connect signal
    g_signal_connect(dropdown, "notify::selected", G_CALLBACK(on_difficulty_changed), NULL);

    // Add widgets to content box
    gtk_box_append(GTK_BOX(content_box), label);
    gtk_box_append(GTK_BOX(content_box), dropdown);

    // Add content box to dialog
    gtk_box_append(GTK_BOX(content_area), content_box);

    // Add margin to button area
    GtkWidget* action_area = gtk_widget_get_last_child(GTK_WIDGET(dialog));
    if (action_area)
    {
        gtk_widget_set_margin_top(action_area, 10);
        gtk_widget_set_margin_bottom(action_area, 10);
        gtk_widget_set_margin_start(action_area, 10);
        gtk_widget_set_margin_end(action_area, 10);
    }

    // Show dialog
    gtk_widget_set_visible(dialog, TRUE);

    // Connect close signal
    g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_destroy), NULL);
}

static GtkWidget* create_button_box(GtkWidget* grid, GtkWidget* window)
{
    GtkWidget* button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_start(button_box, OUTER_MARGIN);
    gtk_widget_set_margin_end(button_box, OUTER_MARGIN);
    gtk_widget_set_margin_bottom(button_box, OUTER_MARGIN);
    gtk_widget_set_hexpand(button_box, TRUE);
    gtk_box_set_homogeneous(GTK_BOX(button_box), FALSE);

    GtkWidget* button1 = gtk_button_new_with_label("Generate Board");
    GtkWidget* button2 = gtk_button_new_with_label("Solve Board");
    GtkWidget* button3 = gtk_button_new_with_label("Clear Board");
    GtkWidget* button4 = gtk_button_new_with_label("Settings");

    gtk_widget_set_hexpand(button1, TRUE);
    gtk_widget_set_hexpand(button2, TRUE);
    gtk_widget_set_hexpand(button3, TRUE);
    gtk_widget_set_hexpand(button4, FALSE);

    gtk_box_append(GTK_BOX(button_box), button1);
    gtk_box_append(GTK_BOX(button_box), button2);
    gtk_box_append(GTK_BOX(button_box), button3);
    gtk_box_append(GTK_BOX(button_box), button4);

    g_signal_connect(button1, "clicked", G_CALLBACK(on_generate_board_clicked), grid);
    g_signal_connect(button2, "clicked", G_CALLBACK(on_solve_board_clicked), grid);
    g_signal_connect(button3, "clicked", G_CALLBACK(on_clear_board_clicked), grid);
    g_signal_connect(button4, "clicked", G_CALLBACK(on_settings_clicked), window);

    return button_box;
}

static GtkWidget* create_main_window(GtkApplication* app)
{
    GtkWidget* window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Sudoku Suite");

    int window_size = (GRID_SIZE * CELL_SIZE) + (GRID_SIZE * OUTER_MARGIN);
    gtk_window_set_default_size(GTK_WINDOW(window), window_size, window_size);
    return window;
}

static void activate(GtkApplication* app, gpointer user_data)
{
    GtkWidget* window = create_main_window(app);
    load_css_styling();

    GtkWidget* main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget* grid = create_sudoku_grid();
    GtkWidget* button_box = create_button_box(grid, window);

    gtk_box_append(GTK_BOX(main_box), grid);
    gtk_box_append(GTK_BOX(main_box), button_box);

    gtk_window_set_child(GTK_WINDOW(window), main_box);
    gtk_window_present(GTK_WINDOW(window));
}

#ifndef DEBUG_ACTIVE
int main(int argc, char** argv)
{
    srand((unsigned int)time(NULL));

    GtkApplication* app;
    int status;

    app = gtk_application_new("com.example.SudokuSuite", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
#endif

