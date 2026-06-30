#include "../include/minios.h"

static GtkWidget *play_btn, *song_label, *vol_scale;
static char playlist[16][256];
static int pl_cnt = 0, cur_song = 0, playing = 0;
static pid_t player_pid = 0;

static void update_label() {
    if (pl_cnt > 0)
        gtk_label_set_text(GTK_LABEL(song_label), playlist[cur_song]);
    else
        gtk_label_set_text(GTK_LABEL(song_label), "No songs");
}

static void stop_playback() {
    if (player_pid > 0) {
        kill(player_pid, SIGTERM);
        player_pid = 0;
    }
    playing = 0;
}

static void on_destroy() {
    stop_playback();
    gtk_main_quit();
}

static void start_playback() {
    if (pl_cnt == 0) return;
    stop_playback();
    player_pid = fork();
    if (player_pid == 0) {
        execlp("paplay", "paplay", playlist[cur_song], NULL);
        exit(0);
    }
    playing = 1;
}

static void toggle_play(GtkWidget *w, gpointer d) {
    if (playing) stop_playback();
    else start_playback();
    gtk_button_set_label(GTK_BUTTON(play_btn), playing ? "Pause" : "Play");
}

static void next_song(GtkWidget *w, gpointer d) {
    cur_song = (cur_song + 1) % pl_cnt;
    update_label();
    if (playing) start_playback();
}

static void prev_song(GtkWidget *w, gpointer d) {
    cur_song = (cur_song - 1 + pl_cnt) % pl_cnt;
    update_label();
    if (playing) start_playback();
}

static void add_song(GtkWidget *w, gpointer d) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Add Song", NULL, GTK_FILE_CHOOSER_ACTION_OPEN, "_Cancel", GTK_RESPONSE_CANCEL, "_Add", GTK_RESPONSE_ACCEPT, NULL);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT && pl_cnt < 16) {
        char *fn = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        strcpy(playlist[pl_cnt++], fn);
        g_free(fn);
        update_label();
    }
    gtk_widget_destroy(dialog);
}

void launch_musicplayer(int pid) {
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), "Music Player");
    gtk_window_set_default_size(GTK_WINDOW(win), 350, 150);
    g_signal_connect(win, "destroy", G_CALLBACK(on_destroy), NULL);
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(win), vbox);
    song_label = gtk_label_new("No songs");
    gtk_box_pack_start(GTK_BOX(vbox), song_label, FALSE, FALSE, 10);
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *prev_btn = gtk_button_new_with_label("<<");
    play_btn = gtk_button_new_with_label("Play");
    GtkWidget *next_btn = gtk_button_new_with_label(">>");
    GtkWidget *add_btn = gtk_button_new_with_label("+");
    g_signal_connect(prev_btn, "clicked", G_CALLBACK(prev_song), NULL);
    g_signal_connect(play_btn, "clicked", G_CALLBACK(toggle_play), NULL);
    g_signal_connect(next_btn, "clicked", G_CALLBACK(next_song), NULL);
    g_signal_connect(add_btn, "clicked", G_CALLBACK(add_song), NULL);
    gtk_box_pack_start(GTK_BOX(hbox), prev_btn, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), play_btn, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), next_btn, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(hbox), add_btn, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
    vol_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 5);
    gtk_range_set_value(GTK_RANGE(vol_scale), 50);
    gtk_box_pack_start(GTK_BOX(vbox), vol_scale, FALSE, FALSE, 5);
    gtk_widget_show_all(win);
}
