#include "../include/minios.h"

static GtkWidget *src_entry, *dst_entry, *prog_bar, *log_view;

static void* copy_thread(void* arg) {
    char** paths = (char**)arg;
    FILE *src = fopen(paths[0], "rb");
    FILE *dst = fopen(paths[1], "wb");
    if (src && dst) {
        fseek(src, 0, SEEK_END);
        long total = ftell(src);
        fseek(src, 0, SEEK_SET);
        char buf[4096];
        long done = 0;
        size_t n;
        while ((n = fread(buf, 1, 4096, src)) > 0) {
            fwrite(buf, 1, n, dst);
            done += n;
            double frac = (double)done / total;
            g_idle_add((GSourceFunc)gtk_progress_bar_set_fraction, prog_bar);
        }
        fclose(src);
        fclose(dst);
    }
    free(paths[0]);
    free(paths[1]);
    free(paths);
    return NULL;
}

static void do_copy(GtkWidget *w, gpointer d) {
    char** paths = malloc(2 * sizeof(char*));
    paths[0] = strdup(gtk_entry_get_text(GTK_ENTRY(src_entry)));
    paths[1] = strdup(gtk_entry_get_text(GTK_ENTRY(dst_entry)));
    pthread_t t;
    pthread_create(&t, NULL, copy_thread, paths);
    pthread_detach(t);
}

static void do_delete(GtkWidget *w, gpointer d) {
    const char *path = gtk_entry_get_text(GTK_ENTRY(src_entry));
    if (remove(path) == 0) {
        GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(log_view));
        GtkTextIter end;
        gtk_text_buffer_get_end_iter(buf, &end);
        char msg[256];
        snprintf(msg, 256, "Deleted: %s\n", path);
        gtk_text_buffer_insert(buf, &end, msg, -1);
    }
}

void launch_fileops(int pid) {
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "File Operations");
    gtk_window_set_default_size(GTK_WINDOW(win), 400, 250);
    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(win), vbox);
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Source:"), 0, 0, 1, 1);
    src_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), src_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Dest:"), 0, 1, 1, 1);
    dst_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), dst_entry, 1, 1, 1, 1);
    gtk_box_pack_start(GTK_BOX(vbox), grid, FALSE, FALSE, 5);
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *copy_btn = gtk_button_new_with_label("Copy");
    GtkWidget *del_btn = gtk_button_new_with_label("Delete");
    g_signal_connect(copy_btn, "clicked", G_CALLBACK(do_copy), NULL);
    g_signal_connect(del_btn, "clicked", G_CALLBACK(do_delete), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), copy_btn, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), del_btn, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);
    prog_bar = gtk_progress_bar_new();
    gtk_box_pack_start(GTK_BOX(vbox), prog_bar, FALSE, FALSE, 2);
    log_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(log_view), FALSE);
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), log_view);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 2);
    gtk_widget_show_all(win);
}
