#include "../include/minios.h"

/*
 * Multi-Level Queue Scheduler
 *   Level 1  (priority 1) — High   — FCFS (no preemption)
 *   Level 2  (priority 2) — Normal — Round Robin, TQ = 5 ms
 *   Level 3  (priority 3) — Low    — Round Robin, TQ = 10 ms
 *
 * Synchronization:
 *   sched_mutex  protects all three queues.
 *   sched_cond   is signalled whenever a process is enqueued;
 *                the dispatcher thread waits on it when all queues are empty
 *                (no busy-waiting).
 */

pthread_cond_t  sched_cond  = PTHREAD_COND_INITIALIZER;

static Process *q_high[MAX_PROC];
static Process *q_med [MAX_PROC];
static Process *q_low [MAX_PROC];
static int qh = 0, qm = 0, ql = 0;

/* ── enqueue ─────────────────────────────────────────────────────────────── */
void add_to_queue(Process *p) {
    pthread_mutex_lock(&sched_mutex);
    if      (p->priority == 1 && qh < MAX_PROC) q_high[qh++] = p;
    else if (p->priority == 2 && qm < MAX_PROC) q_med [qm++] = p;
    else if (ql < MAX_PROC)                      q_low [ql++] = p;
    pthread_cond_signal(&sched_cond);   /* wake dispatcher */
    pthread_mutex_unlock(&sched_mutex);
}

/* ── dequeue — always picks highest non-empty level ─────────────────────── */
Process *get_next_process(void) {
    pthread_mutex_lock(&sched_mutex);

    /* wait (without busy-spinning) if all queues empty */
    while (qh == 0 && qm == 0 && ql == 0)
        pthread_cond_wait(&sched_cond, &sched_mutex);

    Process *p = NULL;

    if (qh > 0) {
        /* Level 1: FCFS — take head */
        p = q_high[0];
        for (int i = 0; i < qh - 1; i++) q_high[i] = q_high[i+1];
        qh--;
    } else if (qm > 0) {
        /* Level 2: Round Robin — take head (re-enqueue after quantum) */
        p = q_med[0];
        for (int i = 0; i < qm - 1; i++) q_med[i] = q_med[i+1];
        qm--;
    } else {
        /* Level 3: Round Robin — take head */
        p = q_low[0];
        for (int i = 0; i < ql - 1; i++) q_low[i] = q_low[i+1];
        ql--;
    }

    pthread_mutex_unlock(&sched_mutex);
    return p;
}

/* ── dispatcher thread ───────────────────────────────────────────────────── */
void *scheduler_run(void *arg) {
    (void)arg;
    while (1) {
        Process *p = get_next_process();
        if (!p) continue;

        p->state = RUNNING;

        if (p->priority == 1) {
            /* FCFS: run until it yields (simulated by a longer sleep) */
            usleep(200000);
        } else {
            /* Round Robin: run for one time quantum then re-queue */
            usleep((unsigned)(p->time_slice) * 1000);
        }

        /* context switch: save state (simulated), re-enqueue if still alive */
        pthread_mutex_lock(&sched_mutex);
        if (p->state == RUNNING) {
            p->state = READY;
            /* Round Robin levels re-enter the queue at the tail */
            if (p->priority == 2 && qm < MAX_PROC) q_med [qm++] = p;
            else if (p->priority == 3 && ql < MAX_PROC) q_low[ql++] = p;
            /* Level-1 FCFS processes are not re-queued automatically */
        }
        pthread_mutex_unlock(&sched_mutex);
    }
    return NULL;
}
