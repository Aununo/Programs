#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_PHILOSOPHERS 5

pthread_mutex_t chopsticks[NUM_PHILOSOPHERS];


void* philosopher(void* num) {
    int id = *(int*)num;

    while (1) {
        printf("哲学家 %d 正在思考。\n", id);
        sleep(rand() % 3 + 1); 

        printf("哲学家 %d 想要拿起左边的筷子 %d。\n", id, id);
        pthread_mutex_lock(&chopsticks[id]);
        printf("哲学家 %d 已经拿起了左边的筷子 %d。\n", id, id);

        sleep(5);

        printf("哲学家 %d 想要拿起右边的筷子 %d。\n", id, (id + 1) % NUM_PHILOSOPHERS);
        pthread_mutex_lock(&chopsticks[(id + 1) % NUM_PHILOSOPHERS]);
        printf("哲学家 %d 已经拿起了右边的筷子 %d。\n", id, (id + 1) % NUM_PHILOSOPHERS);

        printf("哲学家 %d 正在进餐。\n", id);
        sleep(rand() % 3 + 1);

        pthread_mutex_unlock(&chopsticks[id]);
        printf("哲学家 %d 放下了左边的筷子 %d。\n", id, id);

        pthread_mutex_unlock(&chopsticks[(id + 1) % NUM_PHILOSOPHERS]);
        printf("哲学家 %d 放下了右边的筷子 %d。\n", id, (id + 1) % NUM_PHILOSOPHERS);
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