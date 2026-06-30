#include "../include/minios.h"

/*
 * Kernel Mode Console
 * Only accessible when shared_data->mode == KERNEL_MODE.
 * Allows:
 *   - View all processes + resource table
 *   - Force-kill any process by PID
 *   - Switch back to User Mode
 */

static GtkWidget *log_view;
static GtkWidget *pid_entry;

static void append_log(const char *msg) {
    GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(log_view));
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buf, &end);
    gtk_text_buffer_insert(buf, &end, msg, -1);
    gtk_text_buffer_insert(buf, &end, "\n", -1);
}

static void refresh_procs(GtkWidget *w, gpointer d) {
    if (!shared_data) { append_log("[ERR] No shared memory."); return; }

    char line[256];
    snprintf(line, 256,
             "\n[KERNEL] Resource Table — RAM:%d/%d MB  HDD:%d/%d MB  Cores:%d/%d",
             shared_data->sys_res.total_ram - shared_data->sys_res.avail_ram,
             shared_data->sys_res.total_ram,
             shared_data->sys_res.total_hdd - shared_data->sys_res.avail_hdd,
             shared_data->sys_res.total_hdd,
             shared_data->sys_res.total_cores - shared_data->sys_res.avail_cores,
             shared_data->sys_res.total_cores);
    append_log(line);
    append_log("PID  NAME                 STATE   RAM  HDD  PRIO");
    append_log("─────────────────────────────────────────────────");

    reader_lock();
    for (int i = 0; i < shared_data->proc_count; i++) {
        Process *p = &shared_data->procs[i];
        const char *states[] = {"NEW","READY","RUNNING","BLOCKED","TERMINATED"};
        snprintf(line, 256, "%-4d %-20s %-7s %4dMB %4dMB  %d",
                 p->pid, p->name,
                 (p->state <= TERMINATED) ? states[p->state] : "?",
                 p->mem_req, p->hdd_req, p->priority);
        append_log(line);
    }
    reader_unlock();
}

static void force_kill(GtkWidget *w, gpointer d) {
    if (!shared_data) { append_log("[ERR] No shared memory."); return; }
    if (shared_data->mode != KERNEL_MODE) {
        append_log("[ERR] Must be in Kernel Mode to force-kill.");
        return;
    }
    const char *txt = gtk_entry_get_text(GTK_ENTRY(pid_entry));
    int pid = atoi(txt);
    if (pid <= 0) { append_log("[ERR] Invalid PID."); return; }

    char msg[128];
    Process *p = get_process(pid);
    if (!p) {
        snprintf(msg, 128, "[ERR] PID %d not found.", pid);
        append_log(msg);
        return;
    }
    snprintf(msg, 128, "[KERNEL] Force-killing PID %d ('%s')...", pid, p->name);
    append_log(msg);
    terminate_process(pid);
    append_log("[KERNEL] Done. Resources freed.");
    gtk_entry_set_text(GTK_ENTRY(pid_entry), "");
}

static void toggle_mode(GtkWidget *w, gpointer d) {
    if (!shared_data) return;
    if (shared_data->mode == KERNEL_MODE) {
        shared_data->mode = USER_MODE;
        append_log("[MODE] Switched to User Mode.");
        gtk_button_set_label(GTK_BUTTON(w), "Switch to Kernel Mode");
    } else {
        shared_data->mode = KERNEL_MODE;
        append_log("[MODE] Switched to Kernel Mode.");
        gtk_button_set_label(GTK_BUTTON(w), "Switch to User Mode");
    }
}

void launch_kernelconsole(int pid) {
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "Kernel Mode Console  —  Toji OS");
    gtk_window_set_default_size(GTK_WINDOW(win), 600, 420);
    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    /* title */
    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title),
        "<span foreground='red' weight='bold' size='large'>"
        "⚠  KERNEL MODE CONSOLE  ⚠</span>");
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 4);

    /* log area */
    log_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(log_view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(log_view), TRUE);
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scroll, -1, 260);
    gtk_container_add(GTK_CONTAINER(scroll), log_view);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 2);

    /* PID kill row */
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new("PID to kill:"), FALSE, FALSE, 2);
    pid_entry = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(pid_entry), 8);
    gtk_box_pack_start(GTK_BOX(hbox), pid_entry, FALSE, FALSE, 2);
    GtkWidget *kill_btn = gtk_button_new_with_label("Force Kill");
    g_signal_connect(kill_btn, "clicked", G_CALLBACK(force_kill), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), kill_btn, FALSE, FALSE, 2);

    /* buttons row */
    GtkWidget *hbox2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_pack_start(GTK_BOX(vbox), hbox2, FALSE, FALSE, 2);

    GtkWidget *ref_btn = gtk_button_new_with_label("Refresh Process List");
    g_signal_connect(ref_btn, "clicked", G_CALLBACK(refresh_procs), NULL);
    gtk_box_pack_start(GTK_BOX(hbox2), ref_btn, TRUE, TRUE, 2);

    const char *mode_lbl = (shared_data && shared_data->mode == KERNEL_MODE)
                           ? "Switch to User Mode" : "Switch to Kernel Mode";
    GtkWidget *mode_btn = gtk_button_new_with_label(mode_lbl);
    g_signal_connect(mode_btn, "clicked", G_CALLBACK(toggle_mode), NULL);
    gtk_box_pack_start(GTK_BOX(hbox2), mode_btn, TRUE, TRUE, 2);

    gtk_widget_show_all(win);

    /* auto-enter kernel mode when this app opens */
    if (shared_data) {
        shared_data->mode = KERNEL_MODE;
        gtk_button_set_label(GTK_BUTTON(mode_btn), "Switch to User Mode");
    }
    append_log("[KERNEL] Console open. Kernel Mode active.");
    refresh_procs(NULL, NULL);
}
