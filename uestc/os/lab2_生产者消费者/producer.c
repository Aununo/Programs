#include "common.h"

int main() {
    int sem_id, shm_id;
    struct BufferPool* pool;
    
    srand(time(NULL) ^ getpid()); // 为不同进程初始化不同的随机种子

    // 获取已创建的信号量和共享内存
    sem_id = semget((key_t)SEM_KEY, 1, 0666);
    if (sem_id == -1) {
        perror("Producer: semget failed");
        exit(EXIT_FAILURE);
    }
    shm_id = shmget((key_t)SHM_KEY, sizeof(struct BufferPool), 0666);
    if (shm_id == -1) {
        perror("Producer: shmget failed");
        exit(EXIT_FAILURE);
    }

    // 连接共享内存
    void* shm = shmat(shm_id, NULL, 0);
    if (shm == (void*)-1) {
        perror("Producer: shmat failed");
        exit(EXIT_FAILURE);
    }
    pool = (struct BufferPool*)shm;

    printf("Producer (PID: %d) started.\n", getpid());

    while (1) {
        semaphore_p(sem_id); // P操作，进入临界区 [cite: 233]

        int found_slot = -1;
        // 查找空缓冲区 [cite: 234, 235]
        for (int i = 0; i < BUFFER_SIZE; i++) {
            if (pool->Index[i] == 0) { // 0表示空 [cite: 205]
                found_slot = i;
                break;
            }
        }
        
        if (found_slot != -1) {
            // 将数据放入缓冲区 [cite: 236]
            sprintf(pool->Buffer[found_slot], "Item produced by PID %d in slot %d", getpid(), found_slot);
            pool->Index[found_slot] = 1; // 标记为满 [cite: 236]
            printf("Producer (PID: %d) produced an item in slot %d: '%s'\n", getpid(), found_slot, pool->Buffer[found_slot]);
        } else {
            printf("Producer (PID: %d): Buffer is full, waiting...\n", getpid());
        }

        semaphore_v(sem_id); // V操作，退出临界区 [cite: 240]

        sleep(rand() % 3 + 1); // 随机休眠一段时间 [cite: 241]
    }

    // 理论上不会执行到这里
    shmdt(shm);
    exit(EXIT_SUCCESS);
}