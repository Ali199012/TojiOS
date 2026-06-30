#include "../include/minios.h"

typedef struct { char name[64]; int hour; int min; int active; } SchedTask;
static SchedTask tasks[16];
static int task_cnt = 0;
static GtkWidget *task_list, *name_entry, *hour_spin, *min_spin;

static gboolean check_tasks(gpointer d) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    for (int i = 0; i < task_cnt; i++) {
        if (tasks[i].active && tasks[i].hour == tm->tm_hour && tasks[i].min == tm->tm_min) {
            tasks[i].active = 0;
            char msg[128];
            snprintf(msg, 128, "Task: %s", tasks[i].name);
            GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg);
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
        }
    }
    return TRUE;
}

static void refresh_list() {
    GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(task_list));
    GString *s = g_string_new("Scheduled Tasks:\n");
    for (int i = 0; i < task_cnt; i++)
        g_string_append_printf(s, "%02d:%02d - %s %s\n", tasks[i].hour, tasks[i].min, tasks[i].name, tasks[i].active ? "[active]" : "[done]");
    gtk_text_buffer_set_text(buf, s->str, -1);
    g_string_free(s, TRUE);
}

static void add_task(GtkWidget *w, gpointer d) {
    if (task_cnt < 16) {
        strncpy(tasks[task_cnt].name, gtk_entry_get_text(GTK_ENTRY(name_entry)), 63);
        tasks[task_cnt].hour = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(hour_spin));
        tasks[task_cnt].min = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(min_spin));
        tasks[task_cnt].active = 1;
        task_cnt++;
        refresh_list();
    }
}

void launch_taskscheduler(int pid) {
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "Task Scheduler");
    gtk_window_set_default_size(GTK_WINDOW(win), 350, 300);
    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(win), vbox);
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Task:"), 0, 0, 1, 1);
    name_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), name_entry, 1, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Hour:"), 0, 1, 1, 1);
    hour_spin = gtk_spin_button_new_with_range(0, 23, 1);
    gtk_grid_attach(GTK_GRID(grid), hour_spin, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Min:"), 0, 2, 1, 1);
    min_spin = gtk_spin_button_new_with_range(0, 59, 1);
    gtk_grid_attach(GTK_GRID(grid), min_spin, 1, 2, 1, 1);
    GtkWidget *add_btn = gtk_button_new_with_label("Schedule");
    g_signal_connect(add_btn, "clicked", G_CALLBACK(add_task), NULL);
    gtk_grid_attach(GTK_GRID(grid), add_btn, 2, 1, 1, 2);
    gtk_box_pack_start(GTK_BOX(vbox), grid, FALSE, FALSE, 5);
    task_list = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(task_list), FALSE);
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), task_list);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 2);
    g_timeout_add(60000, check_tasks, NULL);
    refresh_list();
    gtk_widget_show_all(win);
}
