#include "../include/minios.h"

static GtkWidget *plist;
static GtkListStore *store;

static void refresh_list(GtkWidget *w, gpointer d) {
    gtk_list_store_clear(store);
    if (!shared_data) return;
    GtkTreeIter iter;
    for (int i = 0; i < shared_data->proc_count; i++) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, shared_data->procs[i].pid, 1, shared_data->procs[i].name, 2, shared_data->procs[i].state, 3, shared_data->procs[i].mem_req, -1);
    }
}

static void kill_selected(GtkWidget *w, gpointer d) {
    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(plist));
    GtkTreeIter iter;
    GtkTreeModel *model;
    if (gtk_tree_selection_get_selected(sel, &model, &iter)) {
        int pid;
        gtk_tree_model_get(model, &iter, 0, &pid, -1);
        terminate_process(pid);
        refresh_list(NULL, NULL);
    }
}

void launch_procmanager(int pid) {
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "Process Manager");
    gtk_window_set_default_size(GTK_WINDOW(win), 450, 300);
    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(win), vbox);
    store = gtk_list_store_new(4, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT);
    plist = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    GtkCellRenderer *r = gtk_cell_renderer_text_new();
    gtk_tree_view_append_column(GTK_TREE_VIEW(plist), gtk_tree_view_column_new_with_attributes("PID", r, "text", 0, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(plist), gtk_tree_view_column_new_with_attributes("Name", r, "text", 1, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(plist), gtk_tree_view_column_new_with_attributes("State", r, "text", 2, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(plist), gtk_tree_view_column_new_with_attributes("Memory", r, "text", 3, NULL));
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), plist);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 2);
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *ref_btn = gtk_button_new_with_label("Refresh");
    GtkWidget *kill_btn = gtk_button_new_with_label("Kill Process");
    g_signal_connect(ref_btn, "clicked", G_CALLBACK(refresh_list), NULL);
    g_signal_connect(kill_btn, "clicked", G_CALLBACK(kill_selected), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), ref_btn, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), kill_btn, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);
    refresh_list(NULL, NULL);
    gtk_widget_show_all(win);
}
