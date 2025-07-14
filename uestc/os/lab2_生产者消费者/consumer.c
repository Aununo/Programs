#include "common.h"

int main() {
    int sem_id, shm_id;
    struct BufferPool* pool;

    srand(time(NULL) ^ getpid());

    // 获取已创建的信号量和共享内存
    sem_id = semget((key_t)SEM_KEY, 1, 0666);
    if (sem_id == -1) {
        perror("Consumer: semget failed");
        exit(EXIT_FAILURE);
    }
    shm_id = shmget((key_t)SHM_KEY, sizeof(struct BufferPool), 0666);
    if (shm_id == -1) {
        perror("Consumer: shmget failed");
        exit(EXIT_FAILURE);
    }

    // 连接共享内存
    void* shm = shmat(shm_id, NULL, 0);
    if (shm == (void*)-1) {
        perror("Consumer: shmat failed");
        exit(EXIT_FAILURE);
    }
    pool = (struct BufferPool*)shm;

    printf("Consumer (PID: %d) started.\n", getpid());

    while (1) {
        semaphore_p(sem_id); // P操作，进入临界区 [cite: 248]

        int found_slot = -1;
        char consumed_item[TEXT_SIZE];
        // 查找满的缓冲区 [cite: 249, 250]
        for (int i = 0; i < BUFFER_SIZE; i++) {
            if (pool->Index[i] == 1) { // 1表示满 [cite: 205]
                found_slot = i;
                break;
            }
        }

        if (found_slot != -1) {
            // 从缓冲区取走数据 [cite: 251]
            strcpy(consumed_item, pool->Buffer[found_slot]);
            pool->Index[found_slot] = 0; // 标记为空 [cite: 247]
            printf("Consumer (PID: %d) consumed an item from slot %d: '%s'\n", getpid(), found_slot, consumed_item);
        } else {
            printf("Consumer (PID: %d): Buffer is empty, waiting...\n", getpid());
        }

        semaphore_v(sem_id); // V操作，退出临界区 [cite: 255]

        sleep(rand() % 4 + 1); // 随机休眠一段时间 [cite: 256]
    }

    // 理论上不会执行到这里
    shmdt(shm);
    exit(EXIT_SUCCESS);
}