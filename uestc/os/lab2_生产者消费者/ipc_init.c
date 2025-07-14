#include "common.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <init|del>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "init") == 0) {
        // --- 初始化信号量 ---
        int sem_id = semget((key_t)SEM_KEY, 1, 0666 | IPC_CREAT);
        if (sem_id == -1) {
            perror("semget failed");
            exit(EXIT_FAILURE);
        }
        // 初始化为1，用于互斥 [cite: 217]
        if (init_sem(sem_id, 1) == -1) {
            exit(EXIT_FAILURE);
        }
        printf("Semaphore %d created and initialized to 1.\n", sem_id);

        // --- 初始化共享内存 ---
        int shm_id = shmget((key_t)SHM_KEY, sizeof(struct BufferPool), 0666 | IPC_CREAT);
        if (shm_id == -1) {
            perror("shmget failed");
            exit(EXIT_FAILURE);
        }
        void* shm = shmat(shm_id, NULL, 0);
        if (shm == (void*)-1) {
            perror("shmat failed");
            exit(EXIT_FAILURE);
        }
        printf("Shared memory %d created and attached.\n", shm_id);

        // 初始化缓冲池状态
        struct BufferPool* pool = (struct BufferPool*)shm;
        for (int i = 0; i < BUFFER_SIZE; i++) {
            pool->Index[i] = 0; // 0表示缓冲区为空 [cite: 205]
        }
        
        if (shmdt(shm) == -1) {
            perror("shmdt failed");
            exit(EXIT_FAILURE);
        }
        printf("Shared memory initialized and detached.\n");

    } else if (strcmp(argv[1], "del") == 0) {
        // --- 删除信号量 ---
        int sem_id = semget((key_t)SEM_KEY, 1, 0666);
        if (sem_id == -1) {
            perror("semget failed");
        } else if (del_sem(sem_id) != -1) {
            printf("Semaphore %d deleted.\n", sem_id);
        }

        // --- 删除共享内存 ---
        int shm_id = shmget((key_t)SHM_KEY, sizeof(struct BufferPool), 0666);
        if (shm_id == -1) {
            perror("shmget failed");
        } else if (shmctl(shm_id, IPC_RMID, 0) != -1) {
            printf("Shared memory %d deleted.\n", shm_id);
        }

    } else {
        fprintf(stderr, "Usage: %s <init|del>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}