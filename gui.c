#include <gtk/gtk.h>

#define GRID_SIZE 9
#define GRID_BLOCK_SIZE 3
#define OUTER_MARGIN 20
#define CELL_SIZE 48

static void load_css_styling(void)
{
    GtkCssProvider* provider = gtk_css_provider_new();
    GError* err = NULL;
    gtk_css_provider_load_from_path(provider, "gui.css");

    if (err)
    {
        g_warning("Failed to load gui.css: %s", err->message);
        g_clear_error(&err);
    }
    else
    {
        GdkDisplay* display = gdk_display_get_default();
        gtk_style_context_add_provider_for_display(display,
                                                   GTK_STYLE_PROVIDER(provider),
                                                   GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
    g_object_unref(provider);
}

static GtkWidget* create_cell_entry(int row, int col)
{
    GtkWidget* entry = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry), 1);
    gtk_entry_set_alignment(GTK_ENTRY(entry), 0.5);
    gtk_editable_set_position(GTK_EDITABLE(entry), 0);
    gtk_widget_set_size_request(entry, CELL_SIZE, CELL_SIZE);

    GtkStyleContext* sc = gtk_widget_get_style_context(entry);
    gtk_style_context_add_class(sc, "cell");

    /* Add thicker borders on group boundaries */
    if ((col + 1) % GRID_BLOCK_SIZE == 0 && col != GRID_SIZE - 1)
        gtk_style_context_add_class(sc, "thick-right");
    if ((row + 1) % GRID_BLOCK_SIZE == 0 && row != GRID_SIZE - 1)
        gtk_style_context_add_class(sc, "thick-bottom");

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

    /* Create and attach cell entries */
    for (int r = 0; r < GRID_SIZE; ++r)
    {
        for (int c = 0; c < GRID_SIZE; ++c)
        {
            GtkWidget* entry = create_cell_entry(r, c);
            gtk_grid_attach(GTK_GRID(grid), entry, c, r, 1, 1);
        }
    }

    return grid;
}

static GtkWidget* create_main_window(GtkApplication* app)
{
    GtkWidget* window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Sudoku Suite");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 600);
    return window;
}

static void activate(GtkApplication* app, gpointer user_data)
{
    GtkWidget* window = create_main_window(app);
    load_css_styling();
    GtkWidget* grid = create_sudoku_grid();

    gtk_window_set_child(GTK_WINDOW(window), grid);
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char** argv)
{
    GtkApplication* app;
    int status;

    app = gtk_application_new("com.example.SudokuSuite", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
