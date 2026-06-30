#include "../include/minios.h"

SystemResources  sys_res;
Process          procs[MAX_PROC];
int              proc_count  = 0;
SharedData      *shared_data = NULL;
static int       shm_fd      = -1;

/* ── shared memory init ──────────────────────────────────────────────────── */
int init_shared_data(int create) {
    if (create) {
        shm_unlink(SHM_NAME);
        shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    } else {
        shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    }
    if (shm_fd < 0) return -1;
    if (create && ftruncate(shm_fd, sizeof(SharedData)) < 0) return -1;

    shared_data = mmap(NULL, sizeof(SharedData),
                       PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_data == MAP_FAILED) { shared_data = NULL; return -1; }

    if (create) {
        memset(shared_data, 0, sizeof(SharedData));
        shared_data->sys_res.total_ram    = 2048;
        shared_data->sys_res.avail_ram    = 2048;
        shared_data->sys_res.total_hdd    = 256000;
        shared_data->sys_res.avail_hdd    = 256000;
        shared_data->sys_res.total_cores  = 8;
        shared_data->sys_res.avail_cores  = 8;
        shared_data->proc_count           = 0;
        shared_data->next_pid             = 1;
        shared_data->mode                 = USER_MODE;
    }
    return 0;
}

void cleanup_shared_data(void) {
    if (shared_data) { munmap(shared_data, sizeof(SharedData)); shared_data = NULL; }
    if (shm_fd >= 0) { close(shm_fd); shm_fd = -1; }
    shm_unlink(SHM_NAME);
}

/* ── memory manager ──────────────────────────────────────────────────────── */
void init_memory(void) {
    SystemResources *res = shared_data ? &shared_data->sys_res : &sys_res;
    /* values set by boot_sequence via configure_hardware; only fill defaults
       if we somehow got here without them */
    if (res->total_ram == 0)   res->total_ram   = res->avail_ram   = 2048;
    if (res->total_hdd == 0)   res->total_hdd   = res->avail_hdd   = 256000;
    if (res->total_cores == 0) res->total_cores = res->avail_cores = 8;
}

int alloc_ram(int size) {
    SystemResources *res = shared_data ? &shared_data->sys_res : &sys_res;
    pthread_mutex_lock(&res_mutex);
    int ok = (res->avail_ram >= size);
    if (ok) res->avail_ram -= size;
    pthread_mutex_unlock(&res_mutex);
    return ok;
}

void free_ram(int size) {
    SystemResources *res = shared_data ? &shared_data->sys_res : &sys_res;
    pthread_mutex_lock(&res_mutex);
    res->avail_ram += size;
    if (res->avail_ram > res->total_ram) res->avail_ram = res->total_ram;
    pthread_mutex_unlock(&res_mutex);
}

/* ── Banker's algorithm safety check ─────────────────────────────────────── */
int banker_check(int mem, int hdd, int cores) {
    SystemResources *res = shared_data ? &shared_data->sys_res : &sys_res;
    pthread_mutex_lock(&res_mutex);
    int safe = (res->avail_ram >= mem &&
                res->avail_hdd >= hdd &&
                res->avail_cores >= cores);
    pthread_mutex_unlock(&res_mutex);
    return safe;
}
