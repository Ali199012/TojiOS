#include "../include/minios.h"

static GtkWidget     *term_view;
static GtkTextBuffer *term_buf;
static GtkWidget     *cmd_entry;
static GtkWidget     *scroll_win;

/* Auto-scroll to bottom of text view */
static void scroll_to_bottom(void) {
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(term_buf, &end);
    GtkTextMark *mark = gtk_text_buffer_get_mark(term_buf, "insert");
    gtk_text_buffer_place_cursor(term_buf, &end);
    gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(term_view), mark);
}

/* Append text to terminal output */
static void term_print(const char *text) {
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(term_buf, &end);
    gtk_text_buffer_insert(term_buf, &end, text, -1);
}

static void exec_cmd(GtkWidget *w, gpointer d) {
    const char *cmd = gtk_entry_get_text(GTK_ENTRY(cmd_entry));
    if (!cmd || strlen(cmd) == 0) return;

    /* print prompt + command */
    char prompt[512];
    snprintf(prompt, sizeof(prompt), "\n$ %s\n", cmd);
    term_print(prompt);

    /* run command and capture output */
    FILE *fp = popen(cmd, "r");
    if (fp) {
        char line[512];
        while (fgets(line, sizeof(line), fp))
            term_print(line);
        pclose(fp);
    } else {
        term_print("Error: could not run command.\n");
    }

    /* clear input, scroll to bottom, refocus */
    gtk_entry_set_text(GTK_ENTRY(cmd_entry), "");
    scroll_to_bottom();
    gtk_widget_grab_focus(cmd_entry);
}

void launch_terminal(int pid) {
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "Terminal  —  Toji OS");
    gtk_window_set_default_size(GTK_WINDOW(win), 620, 420);
    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    /* output area */
    term_view = gtk_text_view_new();
    term_buf  = gtk_text_view_get_buffer(GTK_TEXT_VIEW(term_view));
    gtk_text_view_set_editable(GTK_TEXT_VIEW(term_view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(term_view), TRUE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(term_view), GTK_WRAP_CHAR);

    /* dark background, green text */
    GdkRGBA bg = {0.05, 0.05, 0.05, 1.0};
    GdkRGBA fg = {0.0,  0.9,  0.0,  1.0};
    gtk_widget_override_background_color(term_view, GTK_STATE_FLAG_NORMAL, &bg);
    gtk_widget_override_color(term_view, GTK_STATE_FLAG_NORMAL, &fg);

    scroll_win = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_win),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
    gtk_container_add(GTK_CONTAINER(scroll_win), term_view);
    gtk_box_pack_start(GTK_BOX(vbox), scroll_win, TRUE, TRUE, 2);

    /* input row */
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);

    GtkWidget *prompt_lbl = gtk_label_new("$");
    gtk_box_pack_start(GTK_BOX(hbox), prompt_lbl, FALSE, FALSE, 4);

    cmd_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(cmd_entry), "Type a command and press Enter...");
    g_signal_connect(cmd_entry, "activate", G_CALLBACK(exec_cmd), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), cmd_entry, TRUE, TRUE, 2);

    GtkWidget *run_btn = gtk_button_new_with_label("Run");
    g_signal_connect(run_btn, "clicked", G_CALLBACK(exec_cmd), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), run_btn, FALSE, FALSE, 2);

    gtk_widget_show_all(win);

    /* welcome message + focus input */
    term_print("Toji OS Terminal\n");
    term_print("────────────────\n");
    term_print("Type a command and press Enter or click Run.\n");
    term_print("Examples: ls, pwd, date, whoami, uname -a\n\n");
    gtk_widget_grab_focus(cmd_entry);
}
