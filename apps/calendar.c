#include "../include/minios.h"

static GtkWidget *cal_widget;
static GtkWidget *event_entry, *event_list;
static char events[32][128];
static int event_cnt = 0;

static void add_event(GtkWidget *w, gpointer d) {
    if (event_cnt < 32) {
        guint y, m, day;
        gtk_calendar_get_date(GTK_CALENDAR(cal_widget), &y, &m, &day);
        const char *txt = gtk_entry_get_text(GTK_ENTRY(event_entry));
        snprintf(events[event_cnt++], 128, "%04d-%02d-%02d: %s", y, m + 1, day, txt);
        GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(event_list));
        GString *s = g_string_new("");
        for (int i = 0; i < event_cnt; i++) g_string_append_printf(s, "%s\n", events[i]);
        gtk_text_buffer_set_text(buf, s->str, -1);
        g_string_free(s, TRUE);
        gtk_entry_set_text(GTK_ENTRY(event_entry), "");
    }
}

void launch_calendar(int pid) {
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "Calendar");
    gtk_window_set_default_size(GTK_WINDOW(win), 350, 400);
    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(win), vbox);
    cal_widget = gtk_calendar_new();
    gtk_box_pack_start(GTK_BOX(vbox), cal_widget, FALSE, FALSE, 2);
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    event_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(event_entry), "Event description");
    GtkWidget *add_btn = gtk_button_new_with_label("Add Event");
    g_signal_connect(add_btn, "clicked", G_CALLBACK(add_event), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), event_entry, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), add_btn, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);
    event_list = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(event_list), FALSE);
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), event_list);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 2);
    gtk_widget_show_all(win);
}
