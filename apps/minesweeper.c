#include "../include/minios.h"

#define ROWS 9
#define COLS 9
#define MINES 10

static int grid[ROWS][COLS], revealed[ROWS][COLS], flags[ROWS][COLS];
static GtkWidget *btns[ROWS][COLS];
static int game_over = 0;

static void init_game() {
    memset(grid, 0, sizeof(grid));
    memset(revealed, 0, sizeof(revealed));
    memset(flags, 0, sizeof(flags));
    game_over = 0;
    srand(time(NULL));
    int placed = 0;
    while (placed < MINES) {
        int r = rand() % ROWS, c = rand() % COLS;
        if (grid[r][c] != -1) { grid[r][c] = -1; placed++; }
    }
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (grid[r][c] == -1) continue;
            int cnt = 0;
            for (int dr = -1; dr <= 1; dr++)
                for (int dc = -1; dc <= 1; dc++)
                    if (r + dr >= 0 && r + dr < ROWS && c + dc >= 0 && c + dc < COLS && grid[r + dr][c + dc] == -1) cnt++;
            grid[r][c] = cnt;
        }
    }
}

static void reveal(int r, int c) {
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS || revealed[r][c]) return;
    revealed[r][c] = 1;
    char txt[4] = "";
    if (grid[r][c] == -1) { strcpy(txt, "*"); game_over = 1; }
    else if (grid[r][c] > 0) sprintf(txt, "%d", grid[r][c]);
    gtk_button_set_label(GTK_BUTTON(btns[r][c]), txt);
    if (grid[r][c] == 0)
        for (int dr = -1; dr <= 1; dr++)
            for (int dc = -1; dc <= 1; dc++) reveal(r + dr, c + dc);
}

static void on_click(GtkWidget *w, gpointer d) {
    if (game_over) return;
    int idx = GPOINTER_TO_INT(d);
    int r = idx / COLS, c = idx % COLS;
    reveal(r, c);
    if (game_over) {
        for (int i = 0; i < ROWS; i++)
            for (int j = 0; j < COLS; j++)
                if (grid[i][j] == -1) gtk_button_set_label(GTK_BUTTON(btns[i][j]), "*");
    }
}

void launch_minesweeper(int pid) {
    init_game();
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "Minesweeper");
    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    GtkWidget *grid_w = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(win), grid_w);
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            btns[r][c] = gtk_button_new_with_label(" ");
            gtk_widget_set_size_request(btns[r][c], 30, 30);
            g_signal_connect(btns[r][c], "clicked", G_CALLBACK(on_click), GINT_TO_POINTER(r * COLS + c));
            gtk_grid_attach(GTK_GRID(grid_w), btns[r][c], c, r, 1, 1);
        }
    }
    gtk_widget_show_all(win);
}
