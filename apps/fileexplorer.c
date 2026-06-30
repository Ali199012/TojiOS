#include "../include/minios.h"

static GtkWidget *flist;
static GtkListStore *fstore;
static char cur_path[512] = "/home";

static void load_dir(const char *path) {
    gtk_list_store_clear(fstore);
    DIR *d = opendir(path);
    if (!d) return;
    struct dirent *ent;
    GtkTreeIter iter;
    while ((ent = readdir(d))) {
        gtk_list_store_append(fstore, &iter);
        char type = (ent->d_type == DT_DIR) ? 'D' : 'F';
        gtk_list_store_set(fstore, &iter, 0, ent->d_name, 1, type, -1);
    }
    closedir(d);
    strcpy(cur_path, path);
}

static void on_row_activated(GtkTreeView *tv, GtkTreePath *p, GtkTreeViewColumn *c, gpointer d) {
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model(tv);
    if (gtk_tree_model_get_iter(model, &iter, p)) {
        char *name;
        char type;
        gtk_tree_model_get(model, &iter, 0, &name, 1, &type, -1);
        if (type == 'D') {
            char newpath[512];
            if (strcmp(name, "..") == 0) {
                char *last = strrchr(cur_path, '/');
                if (last && last != cur_path) *last = 0;
                else strcpy(cur_path, "/");
                load_dir(cur_path);
            } else if (strcmp(name, ".") != 0) {
                snprintf(newpath, 512, "%s/%s", cur_path, name);
                load_dir(newpath);
            }
        }
        g_free(name);
    }
}

void launch_fileexplorer(int pid) {
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "File Explorer");
    gtk_window_set_default_size(GTK_WINDOW(win), 400, 350);
    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(win), vbox);
    fstore = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_CHAR);
    flist = gtk_tree_view_new_with_model(GTK_TREE_MODEL(fstore));
    GtkCellRenderer *r = gtk_cell_renderer_text_new();
    gtk_tree_view_append_column(GTK_TREE_VIEW(flist), gtk_tree_view_column_new_with_attributes("Name", r, "text", 0, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(flist), gtk_tree_view_column_new_with_attributes("Type", r, "text", 1, NULL));
    g_signal_connect(flist, "row-activated", G_CALLBACK(on_row_activated), NULL);
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), flist);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 2);
    load_dir(cur_path);
    gtk_widget_show_all(win);
}
