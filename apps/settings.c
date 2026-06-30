#include "../include/minios.h"

static GtkWidget *ram_spin, *hdd_spin, *core_spin;

static void apply_settings(GtkWidget *w, gpointer d) {
    int new_ram = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ram_spin));
    int new_hdd = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(hdd_spin));
    int new_core = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(core_spin));
    pthread_mutex_lock(&res_mutex);
    int ram_used = sys_res.total_ram - sys_res.avail_ram;
    int hdd_used = sys_res.total_hdd - sys_res.avail_hdd;
    int core_used = sys_res.total_cores - sys_res.avail_cores;
    if (new_ram >= ram_used && new_hdd >= hdd_used && new_core >= core_used) {
        sys_res.total_ram = new_ram;
        sys_res.avail_ram = new_ram - ram_used;
        sys_res.total_hdd = new_hdd;
        sys_res.avail_hdd = new_hdd - hdd_used;
        sys_res.total_cores = new_core;
        sys_res.avail_cores = new_core - core_used;
    }
    pthread_mutex_unlock(&res_mutex);
}

void launch_settings(int pid) {
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "Settings");
    gtk_window_set_default_size(GTK_WINDOW(win), 300, 200);
    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_add(GTK_CONTAINER(win), grid);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("RAM (MB):"), 0, 0, 1, 1);
    ram_spin = gtk_spin_button_new_with_range(512, 16384, 256);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ram_spin), sys_res.total_ram);
    gtk_grid_attach(GTK_GRID(grid), ram_spin, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("HDD (MB):"), 0, 1, 1, 1);
    hdd_spin = gtk_spin_button_new_with_range(10000, 1000000, 1000);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(hdd_spin), sys_res.total_hdd);
    gtk_grid_attach(GTK_GRID(grid), hdd_spin, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("CPU Cores:"), 0, 2, 1, 1);
    core_spin = gtk_spin_button_new_with_range(1, 64, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(core_spin), sys_res.total_cores);
    gtk_grid_attach(GTK_GRID(grid), core_spin, 1, 2, 1, 1);
    GtkWidget *apply_btn = gtk_button_new_with_label("Apply");
    g_signal_connect(apply_btn, "clicked", G_CALLBACK(apply_settings), NULL);
    gtk_grid_attach(GTK_GRID(grid), apply_btn, 0, 3, 2, 1);
    gtk_widget_show_all(win);
}
