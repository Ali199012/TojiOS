#include "../include/minios.h"

void launch_sysinfo(int pid) {
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "System Info");
    gtk_window_set_default_size(GTK_WINDOW(win), 350, 300);
    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    GtkWidget *text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text), FALSE);
    GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
    GString *s = g_string_new("=== MiniOS System Information ===\n\n");
    SystemResources *res = shared_data ? &shared_data->sys_res : &sys_res;
    int pcount = shared_data ? shared_data->proc_count : proc_count;
    g_string_append_printf(s, "Total RAM: %d MB\n", res->total_ram);
    g_string_append_printf(s, "Available RAM: %d MB\n", res->avail_ram);
    g_string_append_printf(s, "Total HDD: %d MB\n", res->total_hdd);
    g_string_append_printf(s, "Available HDD: %d MB\n", res->avail_hdd);
    g_string_append_printf(s, "Total CPU Cores: %d\n", res->total_cores);
    g_string_append_printf(s, "Available Cores: %d\n", res->avail_cores);
    g_string_append_printf(s, "\nRunning Processes: %d\n", pcount);
    struct utsname u;
    if (uname(&u) == 0) {
        g_string_append_printf(s, "\nHost System:\n");
        g_string_append_printf(s, "  OS: %s\n", u.sysname);
        g_string_append_printf(s, "  Release: %s\n", u.release);
        g_string_append_printf(s, "  Machine: %s\n", u.machine);
    }
    gtk_text_buffer_set_text(buf, s->str, -1);
    g_string_free(s, TRUE);
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), text);
    gtk_container_add(GTK_CONTAINER(win), scroll);
    gtk_widget_show_all(win);
}
