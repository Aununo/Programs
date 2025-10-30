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

#define BUFFER_SIZE 5
#define TEXT_SIZE 100

#define SHM_KEY 0x1234
#define SEM_KEY 0x5678

struct BufferPool {
    char Buffer[BUFFER_SIZE][TEXT_SIZE];
    int Index[BUFFER_SIZE];
};

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int init_sem(int sem_id, int init_value);
int del_sem(int sem_id);
int semaphore_p(int sem_id);
int semaphore_v(int sem_id);

#endif