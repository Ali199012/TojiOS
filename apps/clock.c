#include "../include/minios.h"

static GtkWidget *time_labels[4];
static const char* tz_names[] = {"Local", "UTC", "EST", "PST"};
static const int tz_offsets[] = {0, 0, -5, -8};
static GtkWidget *alarm_spin_h, *alarm_spin_m, *alarm_label;
static int alarm_h = -1, alarm_m = -1;

static gboolean update_clocks(gpointer d) {
    time_t t = time(NULL);
    struct tm *local = localtime(&t);
    char buf[32];
    strftime(buf, 32, "%H:%M:%S", local);
    gtk_label_set_text(GTK_LABEL(time_labels[0]), buf);
    struct tm *utc = gmtime(&t);
    strftime(buf, 32, "%H:%M:%S", utc);
    gtk_label_set_text(GTK_LABEL(time_labels[1]), buf);
    int est_h = (utc->tm_hour + 24 - 5) % 24;
    snprintf(buf, 32, "%02d:%02d:%02d", est_h, utc->tm_min, utc->tm_sec);
    gtk_label_set_text(GTK_LABEL(time_labels[2]), buf);
    int pst_h = (utc->tm_hour + 24 - 8) % 24;
    snprintf(buf, 32, "%02d:%02d:%02d", pst_h, utc->tm_min, utc->tm_sec);
    gtk_label_set_text(GTK_LABEL(time_labels[3]), buf);
    if (alarm_h == local->tm_hour && alarm_m == local->tm_min && local->tm_sec == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "ALARM!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        alarm_h = alarm_m = -1;
        gtk_label_set_text(GTK_LABEL(alarm_label), "No alarm set");
    }
    return TRUE;
}

static void set_alarm(GtkWidget *w, gpointer d) {
    alarm_h = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(alarm_spin_h));
    alarm_m = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(alarm_spin_m));
    char buf[32];
    snprintf(buf, 32, "Alarm: %02d:%02d", alarm_h, alarm_m);
    gtk_label_set_text(GTK_LABEL(alarm_label), buf);
}

void launch_clock(int pid) {
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "Clock & Timer");
    gtk_window_set_default_size(GTK_WINDOW(win), 300, 250);
    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(win), vbox);
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    for (int i = 0; i < 4; i++) {
        gtk_grid_attach(GTK_GRID(grid), gtk_label_new(tz_names[i]), 0, i, 1, 1);
        time_labels[i] = gtk_label_new("--:--:--");
        gtk_grid_attach(GTK_GRID(grid), time_labels[i], 1, i, 1, 1);
    }
    gtk_box_pack_start(GTK_BOX(vbox), grid, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 5);
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new("Set Alarm:"), FALSE, FALSE, 2);
    alarm_spin_h = gtk_spin_button_new_with_range(0, 23, 1);
    alarm_spin_m = gtk_spin_button_new_with_range(0, 59, 1);
    gtk_box_pack_start(GTK_BOX(hbox), alarm_spin_h, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(":"), FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), alarm_spin_m, FALSE, FALSE, 2);
    GtkWidget *set_btn = gtk_button_new_with_label("Set");
    g_signal_connect(set_btn, "clicked", G_CALLBACK(set_alarm), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), set_btn, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
    alarm_label = gtk_label_new("No alarm set");
    gtk_box_pack_start(GTK_BOX(vbox), alarm_label, FALSE, FALSE, 5);
    g_timeout_add(1000, update_clocks, NULL);
    gtk_widget_show_all(win);
}
