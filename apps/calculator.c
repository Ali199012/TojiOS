#include "../include/minios.h"

static GtkWidget *display, *hist_view;
static char expr[256] = "";
static char history[2048] = "";

static void append_char(GtkWidget *w, gpointer d) {
    const char *c = (const char*)d;
    strcat(expr, c);
    gtk_entry_set_text(GTK_ENTRY(display), expr);
}

static void clear_expr(GtkWidget *w, gpointer d) {
    expr[0] = 0;
    gtk_entry_set_text(GTK_ENTRY(display), "");
}

static double calc_eval(const char *e) {
    double a = 0, b = 0;
    char op = '+';
    const char *p = e;
    while (*p) {
        if (*p >= '0' && *p <= '9') {
            double n = 0;
            while (*p >= '0' && *p <= '9') n = n * 10 + (*p++ - '0');
            if (*p == '.') { p++; double f = 0.1; while (*p >= '0' && *p <= '9') { n += (*p++ - '0') * f; f *= 0.1; } }
            if (op == '+') a += n; else if (op == '-') a -= n; else if (op == '*') a *= n; else if (op == '/') a /= n;
        } else { op = *p++; }
    }
    return a;
}

static void calculate(GtkWidget *w, gpointer d) {
    double r = calc_eval(expr);
    char res[64];
    snprintf(res, 64, "%.6g", r);
    char line[320];
    snprintf(line, 320, "%s = %s\n", expr, res);
    strcat(history, line);
    GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(hist_view));
    gtk_text_buffer_set_text(buf, history, -1);
    strcpy(expr, res);
    gtk_entry_set_text(GTK_ENTRY(display), expr);
}

void launch_calculator(int pid) {
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "Calculator");
    gtk_window_set_default_size(GTK_WINDOW(win), 300, 400);
    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_container_add(GTK_CONTAINER(win), vbox);
    display = gtk_entry_new();
    gtk_editable_set_editable(GTK_EDITABLE(display), FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), display, FALSE, FALSE, 2);
    const char *btns[] = {"7","8","9","/","4","5","6","*","1","2","3","-","0",".","=","+","C"};
    GtkWidget *grid = gtk_grid_new();
    for (int i = 0; i < 17; i++) {
        GtkWidget *b = gtk_button_new_with_label(btns[i]);
        if (strcmp(btns[i], "=") == 0) g_signal_connect(b, "clicked", G_CALLBACK(calculate), NULL);
        else if (strcmp(btns[i], "C") == 0) g_signal_connect(b, "clicked", G_CALLBACK(clear_expr), NULL);
        else g_signal_connect(b, "clicked", G_CALLBACK(append_char), (gpointer)btns[i]);
        gtk_grid_attach(GTK_GRID(grid), b, i % 4, i / 4, 1, 1);
    }
    gtk_box_pack_start(GTK_BOX(vbox), grid, FALSE, FALSE, 2);
    hist_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(hist_view), FALSE);
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), hist_view);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 2);
    gtk_widget_show_all(win);
}
