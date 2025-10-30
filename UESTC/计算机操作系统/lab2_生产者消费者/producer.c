#include "common.h"

int main() {
    int sem_id, shm_id;
    struct BufferPool* pool;
    
    srand(time(NULL) ^ getpid());

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

    void* shm = shmat(shm_id, NULL, 0);
    if (shm == (void*)-1) {
        perror("Producer: shmat failed");
        exit(EXIT_FAILURE);
    }
    pool = (struct BufferPool*)shm;

    printf("Producer (PID: %d) started.\n", getpid());

    while (1) {
        semaphore_p(sem_id);

        int found_slot = -1;
        for (int i = 0; i < BUFFER_SIZE; i++) {
            if (pool->Index[i] == 0) {
                found_slot = i;
                break;
            }
        }
        
        if (found_slot != -1) {
            sprintf(pool->Buffer[found_slot], "Item produced by PID %d in slot %d", getpid(), found_slot);
            pool->Index[found_slot] = 1;
            printf("Producer (PID: %d) produced an item in slot %d: '%s'\n", getpid(), found_slot, pool->Buffer[found_slot]);
        } else {
            printf("Producer (PID: %d): Buffer is full, waiting...\n", getpid());
        }

        semaphore_v(sem_id);

        sleep(rand() % 3 + 1);
    }

    shmdt(shm);
    exit(EXIT_SUCCESS);
}