#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define NUM_PHILOSOPHERS 5

pthread_mutex_t chopsticks[NUM_PHILOSOPHERS];

void* philosopher(void* num) {
    int id = *(int*)num;
    int left_chopstick = id;
    int right_chopstick = (id + 1) % NUM_PHILOSOPHERS;

    while (1) {
        // 思考
        printf("哲学家 %d 正在思考。\n", id);
        sleep(rand() % 3 + 1);

        // 核心逻辑：根据奇偶编号决定拿筷子的顺序
        if (id % 2 == 0) { // 偶数编号的哲学家
            printf("哲学家 %d (偶数) 准备先拿左筷子 %d, 再拿右筷子 %d。\n", id, left_chopstick, right_chopstick);
            pthread_mutex_lock(&chopsticks[left_chopstick]);
            printf("哲学家 %d 拿起了左筷子 %d。\n", id, left_chopstick);
            pthread_mutex_lock(&chopsticks[right_chopstick]);
            printf("哲学家 %d 拿起了右筷子 %d。\n", id, right_chopstick);
        } else { // 奇数编号的哲学家
            printf("哲学家 %d (奇数) 准备先拿右筷子 %d, 再拿左筷子 %d。\n", id, right_chopstick, left_chopstick);
            pthread_mutex_lock(&chopsticks[right_chopstick]);
            printf("哲学家 %d 拿起了右筷子 %d。\n", id, right_chopstick);
            pthread_mutex_lock(&chopsticks[left_chopstick]);
            printf("哲学家 %d 拿起了左筷子 %d。\n", id, left_chopstick);
        }

        // 进餐
        printf("哲学家 %d 正在进餐。\n", id);
        sleep(rand() % 2 + 1);

        // 进餐完毕，放下筷子（解锁顺序通常与加锁顺序相反，以示良好实践）
        if (id % 2 == 0) {
            pthread_mutex_unlock(&chopsticks[right_chopstick]);
            pthread_mutex_unlock(&chopsticks[left_chopstick]);
        } else {
            pthread_mutex_unlock(&chopsticks[left_chopstick]);
            pthread_mutex_unlock(&chopsticks[right_chopstick]);
        }
        printf("哲学家 %d 放下了所有筷子。\n", id);
    }
    free(num);
    return NULL;
}

int main() {
    pthread_t philosophers[NUM_PHILOSOPHERS];
    int i;
    
    srand(time(NULL)); // 初始化随机数种子

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