/*
 * Toji OS — main.c
 * Phase 3: Process creation (fork/exec), multitasking, resource management
 */
#include "include/minios.h"

typedef struct {
    const char *name;
    void (*launch)(int);
    int mem;       /* MB RAM  */
    int hdd;       /* MB disk */
    int cores;
    int priority;  /* 1=high 2=normal 3=low */
} AppInfo;

static AppInfo apps[] = {
    /* name                  launcher              RAM   HDD  Cores Pri */
    {"System Monitor",       launch_sysmonitor,     64,   10,   1,  1},
    {"Process Manager",      launch_procmanager,    64,   10,   1,  1},
    {"File Explorer",        launch_fileexplorer,  128,   50,   1,  1},
    {"Terminal",             launch_terminal,       64,   10,   1,  1},
    {"Settings",             launch_settings,       32,    5,   1,  1},
    {"Notepad+",             launch_notepad,       128,  100,   1,  2},
    {"Calculator",           launch_calculator,     32,    5,   1,  2},
    {"Calendar",             launch_calendar,       64,   20,   1,  2},
    {"Task Scheduler",       launch_taskscheduler,  64,   10,   1,  2},
    {"File Operations",      launch_fileops,       128,  200,   1,  2},
    {"System Info",          launch_sysinfo,        32,    5,   1,  3},
    {"Search",               launch_search,        128,   50,   1,  2},
    {"Minesweeper",          launch_minesweeper,    64,   10,   1,  2},
    {"Music Player",         launch_musicplayer,   256,  100,   1,  2},
    {"Clock & Timer",        launch_clock,          32,    5,   1,  3},
    {"Snake",                launch_snake,          32,    5,   1,  2},
    {"Kernel Console",       launch_kernelconsole,  64,   10,   1,  1},
};

static int   app_count  = sizeof(apps) / sizeof(apps[0]);
static char *prog_path  = NULL;

/* ── Launch an app as a new process (fork + exec) ────────────────────────── */
static void on_app_click(GtkWidget *w, gpointer data) {
    int idx = GPOINTER_TO_INT(data);

    /* Kernel-mode guard: only Kernel Console accessible in user mode for now */
    if (shared_data && shared_data->mode == USER_MODE &&
        strcmp(apps[idx].name, "Kernel Console") == 0) {
        GtkWidget *dlg = gtk_message_dialog_new(
            NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
            "Kernel Console requires Kernel Mode.\n"
            "Switch mode via the mode button first.");
        gtk_dialog_run(GTK_DIALOG(dlg));
        gtk_widget_destroy(dlg);
        return;
    }

    /* Resource check + PCB creation (also does fork/grant handshake) */
    int proc_pid = create_process(
        apps[idx].name,
        apps[idx].mem, apps[idx].hdd,
        apps[idx].cores, apps[idx].priority);

    if (proc_pid < 0) {
        GtkWidget *dlg = gtk_message_dialog_new(
            NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
            "Resource DENIED for '%s'.\n"
            "Not enough RAM, Disk or CPU cores available.",
            apps[idx].name);
        gtk_dialog_run(GTK_DIALOG(dlg));
        gtk_widget_destroy(dlg);
        return;
    }

    /* Fork a child that exec()s this same binary with --app <idx> */
    pid_t pid = fork();
    if (pid == 0) {
        char idx_str[8];
        snprintf(idx_str, sizeof(idx_str), "%d", idx);
        execl(prog_path, prog_path, "--app", idx_str, NULL);
        perror("execl");
        exit(1);
    }
    /* parent: detach child (let it run independently) */
}

static void on_mode_toggle(GtkWidget *w, gpointer d) {
    if (!shared_data) return;
    if (shared_data->mode == USER_MODE) {
        shared_data->mode = KERNEL_MODE;
        gtk_button_set_label(GTK_BUTTON(w), "Mode: KERNEL");
        printf("\033[1;31m[OS] Switched to KERNEL MODE\033[0m\n");
    } else {
        shared_data->mode = USER_MODE;
        gtk_button_set_label(GTK_BUTTON(w), "Mode: USER");
        printf("\033[1;32m[OS] Switched to USER MODE\033[0m\n");
    }
}

static void on_shutdown(GtkWidget *w, gpointer d) {
    printf("\n\033[1;36m[TOJI OS] Shutting down...\033[0m\n");
    cleanup_shm();
    cleanup_msg_queue();
    cleanup_sync();
    gtk_main_quit();
}

/* ── Desktop window ──────────────────────────────────────────────────────── */
static void create_desktop(void) {
    GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), OS_NAME " — Desktop");
    gtk_window_set_default_size(GTK_WINDOW(win), 860, 520);
    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    /* Title bar */
    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title),
        "<span weight='bold' size='x-large'>"OS_NAME"  v"OS_VERSION"</span>");
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 4);

    gtk_box_pack_start(GTK_BOX(vbox),
                       gtk_separator_new(GTK_ORIENTATION_HORIZONTAL),
                       FALSE, FALSE, 2);

    /* App grid */
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing   (GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    for (int i = 0; i < app_count; i++) {
        GtkWidget *btn = gtk_button_new_with_label(apps[i].name);
        g_signal_connect(btn, "clicked", G_CALLBACK(on_app_click),
                         GINT_TO_POINTER(i));
        gtk_grid_attach(GTK_GRID(grid), btn, i % 6, i / 6, 1, 1);
    }
    gtk_box_pack_start(GTK_BOX(vbox), grid, TRUE, TRUE, 4);

    gtk_box_pack_start(GTK_BOX(vbox),
                       gtk_separator_new(GTK_ORIENTATION_HORIZONTAL),
                       FALSE, FALSE, 2);

    /* Bottom bar */
    GtkWidget *hbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(vbox), hbar, FALSE, FALSE, 2);

    GtkWidget *mode_btn = gtk_button_new_with_label("Mode: USER");
    g_signal_connect(mode_btn, "clicked", G_CALLBACK(on_mode_toggle), NULL);
    gtk_box_pack_start(GTK_BOX(hbar), mode_btn, FALSE, FALSE, 2);

    GtkWidget *shut_btn = gtk_button_new_with_label("⏻  Shutdown");
    g_signal_connect(shut_btn, "clicked", G_CALLBACK(on_shutdown), NULL);
    gtk_box_pack_end(GTK_BOX(hbar), shut_btn, FALSE, FALSE, 2);

    gtk_widget_show_all(win);
}

/* ── App mode: child process runs the chosen app ─────────────────────────── */
static void run_app_mode(int idx) {
    init_shared_data(0);   /* attach to existing shared memory */
    gtk_init(NULL, NULL);
    apps[idx].launch(idx + 1);
    gtk_main();
}

/* ── Entry point ─────────────────────────────────────────────────────────── */
int main(int argc, char **argv) {
    prog_path = argv[0];

    /* child app mode */
    if (argc >= 3 && strcmp(argv[1], "--app") == 0) {
        int idx = atoi(argv[2]);
        if (idx >= 0 && idx < app_count)
            run_app_mode(idx);
        return 0;
    }

    /* ── kernel / OS init ── */
    init_shared_data(1);
    boot_sequence();        /* prints banner, asks hardware config, sets resources */
    init_memory();
    init_sync();
    init_shared_mem(4096);
    init_msg_queue();

    /* start scheduler thread */
    pthread_t sched_thread;
    pthread_create(&sched_thread, NULL, scheduler_run, NULL);
    pthread_detach(sched_thread);

    /* auto-start system processes */
    create_process("System Monitor", 64, 10, 1, 1);
    create_process("Clock & Timer",  32,  5, 1, 3);

    gtk_init(&argc, &argv);
    create_desktop();
    gtk_main();

    cleanup_shared_data();
    return 0;
}
