#include "../include/minios.h"

/* ASCII art banner for Toji OS */
static void print_banner(void) {
    printf("\033[2J\033[H");   /* clear screen */
    printf("\033[1;36m");      /* bold cyan */
    printf("\n");
    printf("  ████████╗ ██████╗      ██╗██╗     ██████╗  ███████╗\n");
    printf("     ██╔══╝██╔═══██╗     ██║██║    ██╔═══██╗ ██╔════╝\n");
    printf("     ██║   ██║   ██║     ██║██║    ██║   ██║ ███████╗\n");
    printf("     ██║   ██║   ██║██   ██║██║    ██║   ██║ ╚════██║\n");
    printf("     ██║   ╚██████╔╝╚█████╔╝██║    ╚██████╔╝ ███████║\n");
    printf("     ╚═╝    ╚═════╝  ╚════╝ ╚═╝     ╚═════╝  ╚══════╝\n");
    printf("\033[0m");
    printf("\033[1;37m           %s  v%s  — Solo Project\033[0m\n\n", OS_NAME, OS_VERSION);
}

static void boot_step(const char *msg, int delay_ms) {
    printf("\033[1;32m[BOOT]\033[0m %s", msg);
    fflush(stdout);
    usleep(delay_ms * 1000);
    printf(" \033[1;32m✓\033[0m\n");
    fflush(stdout);
}

/* Ask user for hardware config and store in shared_data */
static void configure_hardware(void) {
    SystemResources *res = shared_data ? &shared_data->sys_res : &sys_res;
    int ram = 0, hdd = 0, cores = 0;

    printf("\033[1;33m[TOJI OS] Hardware Configuration\033[0m\n");
    printf("─────────────────────────────────────\n");

    while (ram <= 0) {
        printf("  Enter RAM size   (MB, e.g. 2048) : ");
        fflush(stdout);
        if (scanf("%d", &ram) != 1 || ram <= 0) {
            printf("  Invalid. Please enter a positive number.\n");
            ram = 0;
            while(getchar() != '\n');
        }
    }
    while (hdd <= 0) {
        printf("  Enter Disk size  (MB, e.g. 256000): ");
        fflush(stdout);
        if (scanf("%d", &hdd) != 1 || hdd <= 0) {
            printf("  Invalid.\n");
            hdd = 0;
            while(getchar() != '\n');
        }
    }
    while (cores <= 0) {
        printf("  Enter CPU cores  (e.g. 8)        : ");
        fflush(stdout);
        if (scanf("%d", &cores) != 1 || cores <= 0) {
            printf("  Invalid.\n");
            cores = 0;
            while(getchar() != '\n');
        }
    }
    while(getchar() != '\n');   /* flush newline */

    res->total_ram    = ram;
    res->avail_ram    = ram;
    res->total_hdd    = hdd;
    res->avail_hdd    = hdd;
    res->total_cores  = cores;
    res->avail_cores  = cores;

    printf("\n");
    printf("  \033[1;36mRAM   : %d MB\033[0m\n", ram);
    printf("  \033[1;36mDisk  : %d MB\033[0m\n", hdd);
    printf("  \033[1;36mCores : %d\033[0m\n\n",  cores);
    usleep(600000);
}

void boot_sequence(void) {
    print_banner();
    usleep(800000);

    configure_hardware();

    boot_step("Initializing kernel memory manager ...", 350);
    boot_step("Starting IPC subsystem (pipes + msg queues) ...", 300);
    boot_step("Initializing synchronization primitives ...", 250);
    boot_step("Loading multilevel scheduler (FCFS / Round-Robin) ...", 300);
    boot_step("Mounting virtual disk image ...", 350);
    boot_step("Starting background services ...", 300);

    SystemResources *res = shared_data ? &shared_data->sys_res : &sys_res;
    printf("\n\033[1;32m[TOJI OS] System ready!\033[0m\n");
    printf("  RAM: %d MB  |  Disk: %d MB  |  Cores: %d\n\n",
           res->total_ram, res->total_hdd, res->total_cores);
    usleep(600000);
}
