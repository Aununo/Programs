#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_PHILOSOPHERS 5 // 哲学家的数量

pthread_mutex_t chopsticks[NUM_PHILOSOPHERS]; // 为五支筷子定义五个互斥量


void* philosopher(void* num) {
    int id = *(int*)num;

    while (1) {
        printf("哲学家 %d 正在思考。\n", id);
        sleep(rand() % 3 + 1); 

        // 尝试拿起左边的筷子（阻塞式）takechopstick(number)
        printf("哲学家 %d 想要拿起左边的筷子 %d。\n", id, id);
        pthread_mutex_lock(&chopsticks[id]);
        printf("哲学家 %d 已经拿起了左边的筷子 %d。\n", id, id);

        // 为了更容易观察到死锁，在此处短暂延时，增加所有哲学家都拿起左边筷子的可能性
        sleep(5);

        // 尝试拿起右边的筷子（阻塞式）takechopstick((number+1)%5)
        printf("哲学家 %d 想要拿起右边的筷子 %d。\n", id, (id + 1) % NUM_PHILOSOPHERS);
        pthread_mutex_lock(&chopsticks[(id + 1) % NUM_PHILOSOPHERS]);
        printf("哲学家 %d 已经拿起了右边的筷子 %d。\n", id, (id + 1) % NUM_PHILOSOPHERS);

        // 进餐
        printf("哲学家 %d 正在进餐。\n", id);
        sleep(rand() % 3 + 1); // 进餐随机时间

        // 放下左边的筷子
        pthread_mutex_unlock(&chopsticks[id]);
        printf("哲学家 %d 放下了左边的筷子 %d。\n", id, id);

        // 放下右边的筷子
        pthread_mutex_unlock(&chopsticks[(id + 1) % NUM_PHILOSOPHERS]);
        printf("哲学家 %d 放下了右边的筷子 %d。\n", id, (id + 1) % NUM_PHILOSOPHERS);
    }

    free(num);
    return NULL;
}

int main() {
    pthread_t philosophers[NUM_PHILOSOPHERS];
    int i;

    // 初始化互斥量
    for (i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_mutex_init(&chopsticks[i], NULL);
    }

    // 创建哲学家线程
    for (i = 0; i < NUM_PHILOSOPHERS; i++) {
        int* philosopher_id = malloc(sizeof(int));
        *philosopher_id = i;
        pthread_create(&philosophers[i], NULL, philosopher, (void*)philosopher_id);
    }

    // 等待所有哲学家线程（理论上是无限循环，所以这里不会执行到）
    for (i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_join(philosophers[i], NULL);
    }

    // 销毁互斥量
    for (i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_mutex_destroy(&chopsticks[i]);
    }

    return 0;
}