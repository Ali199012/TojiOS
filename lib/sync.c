#include "../include/minios.h"

pthread_mutex_t res_mutex   = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sched_mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t           res_sem;

void init_sync(void) {
    sem_init(&res_sem, 0, 1);
}

void cleanup_sync(void) {
    sem_destroy(&res_sem);
    pthread_mutex_destroy(&res_mutex);
    pthread_mutex_destroy(&sched_mutex);
    pthread_cond_destroy(&sched_cond);
}

void acquire_resource(void) { sem_wait(&res_sem);  }
void release_resource(void) { sem_post(&res_sem);  }

static pthread_rwlock_t rw_lock = PTHREAD_RWLOCK_INITIALIZER;
void reader_lock(void)   { pthread_rwlock_rdlock(&rw_lock); }
void reader_unlock(void) { pthread_rwlock_unlock(&rw_lock); }
void writer_lock(void)   { pthread_rwlock_wrlock(&rw_lock); }
void writer_unlock(void) { pthread_rwlock_unlock(&rw_lock); }
