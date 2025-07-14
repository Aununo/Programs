#include "common.h"

// 初始化信号量 [cite: 378]
int init_sem(int sem_id, int init_value) {
    union semun sem_union;
    sem_union.val = init_value;
    if (semctl(sem_id, 0, SETVAL, sem_union) == -1) {
        perror("Initialize semaphore failed");
        return -1;
    }
    return 1;
}

// 删除信号量集 [cite: 421]
int del_sem(int sem_id) {
    union semun sem_union;
    if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1) {
        perror("Delete semaphore failed");
        return -1;
    }
    return 1;
}

// P操作（申请资源） [cite: 391]
int semaphore_p(int sem_id) {
    struct sembuf sem_b;
    sem_b.sem_num = 0;      // 信号量编号为0
    sem_b.sem_op = -1;      // P操作，值减1 [cite: 395]
    sem_b.sem_flg = SEM_UNDO; // 进程退出时自动撤销操作
    if (semop(sem_id, &sem_b, 1) == -1) {
        perror("P operation failed");
        return 0;
    }
    return 1;
}

// V操作（释放资源） [cite: 406]
int semaphore_v(int sem_id) {
    struct sembuf sem_b;
    sem_b.sem_num = 0;      // 信号量编号为0
    sem_b.sem_op = 1;       // V操作，值加1 [cite: 410]
    sem_b.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem_b, 1) == -1) {
        perror("V operation failed");
        return 0;
    }
    return 1;
}