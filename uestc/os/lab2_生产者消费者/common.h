#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h>

#define BUFFER_SIZE 5       // 缓冲池中缓冲区的数量
#define TEXT_SIZE 100       // 每个缓冲区的大小

// IPC键值，确保不同进程能访问到同一个IPC对象
#define SHM_KEY 0x1234
#define SEM_KEY 0x5678

// 共享缓冲池结构 [cite: 202]
struct BufferPool {
    char Buffer[BUFFER_SIZE][TEXT_SIZE]; // 5个缓冲区，每个100字节 [cite: 204]
    int Index[BUFFER_SIZE];              // 缓冲区状态，0表示空，1表示满 [cite: 205]
};

// 信号量操作联合体，用于semctl调用
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

// 函数声明
int init_sem(int sem_id, int init_value);
int del_sem(int sem_id);
int semaphore_p(int sem_id);
int semaphore_v(int sem_id);

#endif //COMMON_H