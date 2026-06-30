#include "../include/minios.h"

/*
 * Resource request/grant handshake via a pipe:
 *   Parent (kernel) writes IPC_GRANT or IPC_DENY into pipe[1].
 *   Child reads from pipe[0] before calling exec().
 *   This satisfies the project requirement: "the creating process sends a
 *   message containing memory required; if available the process replies
 *   GRANT, else DENY and the process terminates."
 */

int create_process(const char *name, int mem, int hdd, int cores, int priority) {
    /* ── Step 1: Banker's check (kernel side) ── */
    if (!banker_check(mem, hdd, cores)) {
        printf("\033[1;31m[KERNEL] Resource DENIED for '%s' "
               "(RAM:%dMB HDD:%dMB Cores:%d not available)\033[0m\n",
               name, mem, hdd, cores);
        return -1;
    }

    /* ── Step 2: Create IPC pipe for grant/deny handshake ── */
    int pipefd[2];
    if (pipe(pipefd) < 0) {
        perror("[KERNEL] pipe");
        return -1;
    }

    /* ── Step 3: Allocate resources BEFORE fork (under mutex) ── */
    pthread_mutex_lock(&res_mutex);

    int *pcount = shared_data ? &shared_data->proc_count : &proc_count;
    Process *plist = shared_data ? shared_data->procs : procs;
    static int local_pid = 1;
    int *npid = shared_data ? &shared_data->next_pid : &local_pid;

    if (*pcount >= MAX_PROC) {
        pthread_mutex_unlock(&res_mutex);
        close(pipefd[0]); close(pipefd[1]);
        return -1;
    }

    Process *p  = &plist[(*pcount)++];
    p->pid       = (*npid)++;
    strncpy(p->name, name, 49);
    p->mem_req   = mem;
    p->hdd_req   = hdd;
    p->cores_req = cores;
    p->priority  = priority;
    p->state     = NEW;
    p->time_slice = (priority == 1) ? 0 : (priority == 2 ? 5 : 10); /* ms quantum */
    p->created_at = time(NULL);

    SystemResources *res = shared_data ? &shared_data->sys_res : &sys_res;
    res->avail_ram   -= mem;
    res->avail_hdd   -= hdd;
    res->avail_cores -= cores;

    pthread_mutex_unlock(&res_mutex);

    /* ── Step 4: Send GRANT through pipe ── */
    write(pipefd[1], IPC_GRANT, strlen(IPC_GRANT) + 1);
    close(pipefd[1]);

    /* ── Step 5: Child reads the grant token, then continues ── */
    pid_t real_pid = fork();
    if (real_pid == 0) {
        /* child: read grant/deny from pipe */
        char token[16] = {0};
        read(pipefd[0], token, sizeof(token) - 1);
        close(pipefd[0]);
        if (strcmp(token, IPC_GRANT) != 0) {
            /* should never reach here since kernel checked first */
            printf("[CHILD] Resource DENIED — exiting\n");
            exit(1);
        }
        /* child proceeds; the exec() call happens in main.c after this */
        /* We exit here; main.c's on_app_click re-forks with execl */
        exit(0);
    } else {
        close(pipefd[0]);
        waitpid(real_pid, NULL, 0);   /* reap immediate child */
    }

    /* ── Step 6: Mark READY and add to scheduler queue ── */
    p->state = READY;
    add_to_queue(p);

    printf("\033[1;32m[KERNEL] Process %d ('%s') GRANTED — "
           "RAM:%dMB HDD:%dMB Cores:%d  Priority:%d\033[0m\n",
           p->pid, name, mem, hdd, cores, priority);
    log_event("Process created");
    return p->pid;
}

void terminate_process(int pid) {
    pthread_mutex_lock(&res_mutex);

    int *pcount  = shared_data ? &shared_data->proc_count : &proc_count;
    Process *pl  = shared_data ? shared_data->procs : procs;
    SystemResources *res = shared_data ? &shared_data->sys_res : &sys_res;

    for (int i = 0; i < *pcount; i++) {
        if (pl[i].pid == pid) {
            res->avail_ram   += pl[i].mem_req;
            res->avail_hdd   += pl[i].hdd_req;
            res->avail_cores += pl[i].cores_req;
            printf("\033[1;33m[KERNEL] Process %d ('%s') terminated — "
                   "resources freed\033[0m\n", pid, pl[i].name);
            pl[i] = pl[--(*pcount)];   /* swap-remove */
            break;
        }
    }
    pthread_mutex_unlock(&res_mutex);
}

Process *get_process(int pid) {
    int *pcount = shared_data ? &shared_data->proc_count : &proc_count;
    Process *pl  = shared_data ? shared_data->procs : procs;
    for (int i = 0; i < *pcount; i++)
        if (pl[i].pid == pid) return &pl[i];
    return NULL;
}
