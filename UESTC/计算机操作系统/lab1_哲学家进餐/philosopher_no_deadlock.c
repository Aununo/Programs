#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_PHILOSOPHERS 5

pthread_mutex_t chopsticks[NUM_PHILOSOPHERS];

void* philosopher(void* num) {
    int id = *(int*)num;
    int left_chopstick = id;
    int right_chopstick = (id + 1) % NUM_PHILOSOPHERS;

    while (1) {
        printf("哲学家 %d 正在思考。\n", id);
        sleep(rand() % 3 + 1);

        while (1) {
            if (pthread_mutex_trylock(&chopsticks[left_chopstick]) == 0) {
                printf("哲学家 %d 拿起了左边的筷子 %d。\n", id, left_chopstick);
                
                if (pthread_mutex_trylock(&chopsticks[right_chopstick]) == 0) {
                    printf("哲学家 %d 拿起了右边的筷子 %d。\n", id, right_chopstick);
                    break; 
                } else {
                    printf("哲学家 %d 拿不到右筷子 %d, 准备放下左筷子 %d。\n", id, right_chopstick, left_chopstick);
                    pthread_mutex_unlock(&chopsticks[left_chopstick]);
                }
            }
            
            usleep(rand() % 100000 + 100000);
        }

        printf("哲学家 %d 正在进餐。\n", id);
        sleep(rand() % 3 + 1);

        pthread_mutex_unlock(&chopsticks[left_chopstick]);
        pthread_mutex_unlock(&chopsticks[right_chopstick]);
        printf("哲学家 %d 放下了所有筷子。\n", id);
    }

    free(num);
    return NULL;
}

int main() {
    pthread_t philosophers[NUM_PHILOSOPHERS];
    int i;

    for (i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_mutex_init(&chopsticks[i], NULL);
    }

    for (i = 0; i < NUM_PHILOSOPHERS; i++) {
        int* philosopher_id = malloc(sizeof(int));
        *philosopher_id = i;
        pthread_create(&philosophers[i], NULL, philosopher, (void*)philosopher_id);
    }

    for (i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_join(philosophers[i], NULL);
    }

    for (i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_mutex_destroy(&chopsticks[i]);
    }

    return 0;
}