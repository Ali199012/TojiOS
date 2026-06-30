#include "../include/minios.h"

#define ROWS  20
#define COLS  20
#define MAX_SNAKE 400

typedef struct { int r, c; } Point;

static Point snake[MAX_SNAKE];
static int   slen = 3;
static Point food;
static int   dr = 0, dc = 1;   /* direction: start moving right */
static int   running = 1;
static int   score   = 0;

static GtkWidget *canvas;
static GtkWidget *score_label;
static guint      timer_id;

#define CELL 22

static void place_food(void) {
    srand((unsigned)time(NULL) + score);
    int ok = 0;
    while (!ok) {
        food.r = rand() % ROWS;
        food.c = rand() % COLS;
        ok = 1;
        for (int i = 0; i < slen; i++)
            if (snake[i].r == food.r && snake[i].c == food.c) { ok = 0; break; }
    }
}

static gboolean on_draw(GtkWidget *w, cairo_t *cr, gpointer d) {
    /* background */
    cairo_set_source_rgb(cr, 0.05, 0.05, 0.05);
    cairo_paint(cr);

    /* grid */
    cairo_set_source_rgb(cr, 0.12, 0.12, 0.12);
    for (int r = 0; r <= ROWS; r++) {
        cairo_move_to(cr, 0, r * CELL);
        cairo_line_to(cr, COLS * CELL, r * CELL);
    }
    for (int c = 0; c <= COLS; c++) {
        cairo_move_to(cr, c * CELL, 0);
        cairo_line_to(cr, c * CELL, ROWS * CELL);
    }
    cairo_stroke(cr);

    /* food */
    cairo_set_source_rgb(cr, 0.9, 0.2, 0.2);
    cairo_arc(cr, food.c * CELL + CELL/2.0, food.r * CELL + CELL/2.0,
              CELL/2.0 - 2, 0, 2 * G_PI);
    cairo_fill(cr);

    /* snake */
    for (int i = 0; i < slen; i++) {
        double g = (i == 0) ? 0.8 : 0.45;
        cairo_set_source_rgb(cr, 0.1, g, 0.1);
        cairo_rectangle(cr, snake[i].c * CELL + 1, snake[i].r * CELL + 1,
                        CELL - 2, CELL - 2);
        cairo_fill(cr);
    }
    return FALSE;
}

static gboolean game_tick(gpointer d) {
    if (!running) return FALSE;

    /* move body */
    for (int i = slen - 1; i > 0; i--) snake[i] = snake[i-1];
    snake[0].r += dr;
    snake[0].c += dc;

    /* wall collision */
    if (snake[0].r < 0 || snake[0].r >= ROWS ||
        snake[0].c < 0 || snake[0].c >= COLS) {
        running = 0;
        char buf[64];
        snprintf(buf, 64, "Game Over!  Score: %d", score);
        gtk_label_set_text(GTK_LABEL(score_label), buf);
        return FALSE;
    }

    /* self collision */
    for (int i = 1; i < slen; i++) {
        if (snake[i].r == snake[0].r && snake[i].c == snake[0].c) {
            running = 0;
            char buf[64];
            snprintf(buf, 64, "Game Over!  Score: %d", score);
            gtk_label_set_text(GTK_LABEL(score_label), buf);
            return FALSE;
        }
    }

    /* eat food */
    if (snake[0].r == food.r && snake[0].c == food.c) {
        if (slen < MAX_SNAKE) slen++;
        score++;
        char buf[32];
        snprintf(buf, 32, "Score: %d", score);
        gtk_label_set_text(GTK_LABEL(score_label), buf);
        place_food();
    }

    gtk_widget_queue_draw(canvas);
    return TRUE;
}

static gboolean on_key(GtkWidget *w, GdkEventKey *ev, gpointer d) {
    switch (ev->keyval) {
        case GDK_KEY_Up:    if (dr != 1)  { dr=-1; dc=0; } break;
        case GDK_KEY_Down:  if (dr != -1) { dr= 1; dc=0; } break;
        case GDK_KEY_Left:  if (dc != 1)  { dr=0; dc=-1; } break;
        case GDK_KEY_Right: if (dc != -1) { dr=0; dc= 1; } break;
        default: break;
    }
    return TRUE;
}

void launch_snake(int pid) {
    /* init snake */
    snake[0] = (Point){ROWS/2,   COLS/2};
    snake[1] = (Point){ROWS/2,   COLS/2 - 1};
    snake[2] = (Point){ROWS/2,   COLS/2 - 2};
    place_food();

    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "Snake  —  Toji OS");
    gtk_window_set_resizable(GTK_WINDOW(win), FALSE);
    g_signal_connect(win, "destroy",        G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(win, "key-press-event",G_CALLBACK(on_key), NULL);
    gtk_widget_add_events(win, GDK_KEY_PRESS_MASK);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    score_label = gtk_label_new("Score: 0");
    gtk_box_pack_start(GTK_BOX(vbox), score_label, FALSE, FALSE, 4);

    canvas = gtk_drawing_area_new();
    gtk_widget_set_size_request(canvas, COLS * CELL, ROWS * CELL);
    g_signal_connect(canvas, "draw", G_CALLBACK(on_draw), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), canvas, FALSE, FALSE, 0);

    gtk_label_set_text(GTK_LABEL(score_label), "Use arrow keys  |  Score: 0");

    gtk_widget_show_all(win);
    timer_id = g_timeout_add(130, game_tick, NULL);
}
