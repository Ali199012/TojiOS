#include "../include/minios.h"

static GtkWidget *cpu_bar, *ram_bar, *hdd_bar;
static GtkWidget *proc_list;

static gboolean update_monitor(gpointer data) {
    if (!shared_data) return TRUE;
    char buf[64];
    double cpu_use = 0.0, ram_use = 0.0, hdd_use = 0.0;
    SystemResources *res = &shared_data->sys_res;
    if (res->total_cores > 0)
        cpu_use = (double)(res->total_cores - res->avail_cores) / res->total_cores;
    if (res->total_ram > 0)
        ram_use = (double)(res->total_ram - res->avail_ram) / res->total_ram;
    if (res->total_hdd > 0)
        hdd_use = (double)(res->total_hdd - res->avail_hdd) / res->total_hdd;
    if (cpu_use < 0) cpu_use = 0;
    if (cpu_use > 1) cpu_use = 1;
    if (ram_use < 0) ram_use = 0;
    if (ram_use > 1) ram_use = 1;
    if (hdd_use < 0) hdd_use = 0;
    if (hdd_use > 1) hdd_use = 1;
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(cpu_bar), cpu_use);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(ram_bar), ram_use);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(hdd_bar), hdd_use);
    sprintf(buf, "CPU: %.0f%%", cpu_use * 100);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(cpu_bar), buf);
    sprintf(buf, "RAM: %d/%d MB", res->total_ram - res->avail_ram, res->total_ram);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(ram_bar), buf);
    sprintf(buf, "HDD: %d/%d MB", res->total_hdd - res->avail_hdd, res->total_hdd);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(hdd_bar), buf);
    GtkTextBuffer* tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(proc_list));
    GString* s = g_string_new("PID\tNAME\t\tSTATE\tMEM\n");
    for (int i = 0; i < shared_data->proc_count; i++) {
        g_string_append_printf(s, "%d\t%s\t\t%d\t%dMB\n", shared_data->procs[i].pid, shared_data->procs[i].name, shared_data->procs[i].state, shared_data->procs[i].mem_req);
    }
    gtk_text_buffer_set_text(tbuf, s->str, -1);
    g_string_free(s, TRUE);
    return TRUE;
}

void launch_sysmonitor(int pid) {
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "System Monitor");
    gtk_window_set_default_size(GTK_WINDOW(win), 400, 300);
    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(win), vbox);
    cpu_bar = gtk_progress_bar_new();
    ram_bar = gtk_progress_bar_new();
    hdd_bar = gtk_progress_bar_new();
    gtk_widget_set_size_request(cpu_bar, 350, -1);
    gtk_widget_set_size_request(ram_bar, 350, -1);
    gtk_widget_set_size_request(hdd_bar, 350, -1);
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(cpu_bar), TRUE);
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(ram_bar), TRUE);
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(hdd_bar), TRUE);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(cpu_bar), 0.0);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(ram_bar), 0.0);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(hdd_bar), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), cpu_bar, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(vbox), ram_bar, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(vbox), hdd_bar, FALSE, FALSE, 2);
    proc_list = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(proc_list), FALSE);
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), proc_list);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 2);
    update_monitor(NULL);
    g_timeout_add(1000, update_monitor, NULL);
    gtk_widget_show_all(win);
}
