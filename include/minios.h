#ifndef TOJIOS_H
#define TOJIOS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <dirent.h>
#include <gtk/gtk.h>

#define OS_NAME        "Toji OS"
#define OS_VERSION     "1.0"
#define SHM_NAME       "/tojios_shm"
#define MAX_PROC       64
#define IPC_GRANT      "GRANT"
#define IPC_DENY       "DENY"

typedef enum { NEW, READY, RUNNING, BLOCKED, TERMINATED } PState;
typedef enum { USER_MODE = 0, KERNEL_MODE = 1 } OSMode;

typedef struct {
    int    pid;
    char   name[50];
    int    mem_req;
    int    hdd_req;
    int    cores_req;
    int    priority;
    PState state;
    int    time_slice;
    time_t created_at;
} Process;

typedef struct {
    int total_ram,  avail_ram;
    int total_hdd,  avail_hdd;
    int total_cores,avail_cores;
} SystemResources;

typedef struct {
    SystemResources sys_res;
    Process         procs[MAX_PROC];
    int             proc_count;
    int             next_pid;
    OSMode          mode;
} SharedData;

typedef struct {
    long mtype;
    char mtext[256];
} IPCMsg;

extern SharedData      *shared_data;
extern SystemResources  sys_res;
extern Process          procs[MAX_PROC];
extern int              proc_count;
extern pthread_mutex_t  res_mutex;
extern pthread_mutex_t  sched_mutex;
extern sem_t            res_sem;
extern pthread_cond_t   sched_cond;

void     boot_sequence(void);
void     init_memory(void);
int      init_shared_data(int create);
void     cleanup_shared_data(void);
int      alloc_ram(int size);
void     free_ram(int size);
int      banker_check(int mem, int hdd, int cores);
int      create_process(const char *name, int mem, int hdd, int cores, int priority);
void     terminate_process(int pid);
Process *get_process(int pid);
void     add_to_queue(Process *p);
Process *get_next_process(void);
void    *scheduler_run(void *arg);
int      init_shared_mem(int size);
void     write_shm(void *data, int size);
void     read_shm(void *data, int size);
void     cleanup_shm(void);
int      init_msg_queue(void);
int      send_msg(long type, const char *text);
int      recv_msg(long type, char *buf);
void     cleanup_msg_queue(void);
void     init_sync(void);
void     cleanup_sync(void);
void     acquire_resource(void);
void     release_resource(void);
void     reader_lock(void);
void     reader_unlock(void);
void     writer_lock(void);
void     writer_unlock(void);
char    *get_time_str(void);
char    *get_date_str(void);
void     log_event(const char *msg);

void launch_sysmonitor(int pid);
void launch_procmanager(int pid);
void launch_fileexplorer(int pid);
void launch_terminal(int pid);
void launch_settings(int pid);
void launch_notepad(int pid);
void launch_calculator(int pid);
void launch_calendar(int pid);
void launch_taskscheduler(int pid);
void launch_fileops(int pid);
void launch_sysinfo(int pid);
void launch_search(int pid);
void launch_minesweeper(int pid);
void launch_musicplayer(int pid);
void launch_clock(int pid);
void launch_snake(int pid);
void launch_kernelconsole(int pid);

#endif
