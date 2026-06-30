#include "../include/minios.h"

int shm_id = -1;
void* shm_ptr = NULL;
int msg_qid = -1;

int init_shared_mem(int size) {
    shm_id = shmget(IPC_PRIVATE, size, IPC_CREAT | 0666);
    if (shm_id < 0) return -1;
    shm_ptr = shmat(shm_id, NULL, 0);
    return (shm_ptr == (void*)-1) ? -1 : 0;
}

void write_shm(void* data, int size) {
    if (shm_ptr) memcpy(shm_ptr, data, size);
}

void read_shm(void* data, int size) {
    if (shm_ptr) memcpy(data, shm_ptr, size);
}

void cleanup_shm() {
    if (shm_ptr) shmdt(shm_ptr);
    if (shm_id >= 0) shmctl(shm_id, IPC_RMID, NULL);
}

int init_msg_queue() {
    msg_qid = msgget(IPC_PRIVATE, IPC_CREAT | 0666);
    return msg_qid;
}

int send_msg(long type, const char* text) {
    IPCMsg msg;
    msg.mtype = type;
    strncpy(msg.mtext, text, 255);
    return msgsnd(msg_qid, &msg, sizeof(msg.mtext), 0);
}

int recv_msg(long type, char* buf) {
    IPCMsg msg;
    if (msgrcv(msg_qid, &msg, sizeof(msg.mtext), type, IPC_NOWAIT) > 0) {
        strcpy(buf, msg.mtext);
        return 1;
    }
    return 0;
}

void cleanup_msg_queue() {
    if (msg_qid >= 0) msgctl(msg_qid, IPC_RMID, NULL);
}
