#include "../include/minios.h"

static GtkWidget *search_entry, *result_view;

static void* search_thread(void* arg) {
    char* pattern = (char*)arg;
    char cmd[512];
    snprintf(cmd, 512, "find /home -name '*%s*' 2>/dev/null", pattern);
    FILE *fp = popen(cmd, "r");
    GString *res = g_string_new("");
    char line[256];
    while (fgets(line, 256, fp)) g_string_append(res, line);
    pclose(fp);
    GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(result_view));
    gtk_text_buffer_set_text(buf, res->str, -1);
    g_string_free(res, TRUE);
    free(pattern);
    return NULL;
}

static void do_search(GtkWidget *w, gpointer d) {
    char *pattern = strdup(gtk_entry_get_text(GTK_ENTRY(search_entry)));
    pthread_t t;
    pthread_create(&t, NULL, search_thread, pattern);
    pthread_detach(t);
}

void launch_search(int pid) {
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "Search");
    gtk_window_set_default_size(GTK_WINDOW(win), 400, 300);
    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(win), vbox);
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    search_entry = gtk_entry_new();
    GtkWidget *btn = gtk_button_new_with_label("Search");
    g_signal_connect(btn, "clicked", G_CALLBACK(do_search), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), search_entry, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), btn, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
    result_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(result_view), FALSE);
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), result_view);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 2);
    gtk_widget_show_all(win);
}
