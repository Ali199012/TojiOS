#include "../include/minios.h"

static GtkWidget *text_view;
static char file_path[512] = "";
static int app_pid = 0;

static gboolean auto_save(gpointer d) {
    if (strlen(file_path) > 0) {
        GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(buf, &start, &end);
        char *text = gtk_text_buffer_get_text(buf, &start, &end, FALSE);
        FILE *f = fopen(file_path, "w");
        if (f) {
            fputs(text, f);
            fclose(f);
        }
        g_free(text);
    }
    return TRUE;
}

static void open_file(GtkWidget *w, gpointer d) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Open", GTK_WINDOW(d), GTK_FILE_CHOOSER_ACTION_OPEN, "_Cancel", GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, NULL);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *fn = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        strcpy(file_path, fn);
        FILE *f = fopen(fn, "r");
        if (f) {
            fseek(f, 0, SEEK_END);
            long len = ftell(f);
            fseek(f, 0, SEEK_SET);
            char *content = malloc(len + 1);
            fread(content, 1, len, f);
            content[len] = 0;
            fclose(f);
            GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
            gtk_text_buffer_set_text(buf, content, -1);
            free(content);
        }
        g_free(fn);
    }
    gtk_widget_destroy(dialog);
}

static void save_file(GtkWidget *w, gpointer d) {
    if (strlen(file_path) == 0) {
        GtkWidget *dialog = gtk_file_chooser_dialog_new("Save", GTK_WINDOW(d), GTK_FILE_CHOOSER_ACTION_SAVE, "_Cancel", GTK_RESPONSE_CANCEL, "_Save", GTK_RESPONSE_ACCEPT, NULL);
        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
            char *fn = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
            strcpy(file_path, fn);
            g_free(fn);
        }
        gtk_widget_destroy(dialog);
    }
    auto_save(NULL);
}

void launch_notepad(int pid) {
    app_pid = pid;
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "Notepad+");
    gtk_window_set_default_size(GTK_WINDOW(win), 500, 400);
    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_container_add(GTK_CONTAINER(win), vbox);
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    GtkWidget *open_btn = gtk_button_new_with_label("Open");
    GtkWidget *save_btn = gtk_button_new_with_label("Save");
    g_signal_connect(open_btn, "clicked", G_CALLBACK(open_file), win);
    g_signal_connect(save_btn, "clicked", G_CALLBACK(save_file), win);
    gtk_box_pack_start(GTK_BOX(hbox), open_btn, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), save_btn, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);
    text_view = gtk_text_view_new();
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), text_view);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 2);
    g_timeout_add(30000, auto_save, NULL);
    gtk_widget_show_all(win);
}
