#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


#define N 5 //哲学家数量
#define LEFT (id)
#define RIGHT ((id + 1) % N)


pthread_mutex_t chopsticks[N];


void *philosopher(void *arg) {
    int id = *(int *)arg;
    int left = LEFT;
    int right = RIGHT;

    for (int i = 0; i < 10; i++) {
            printf("哲学家 %d 正在思考\n", id);
            sleep(1);

            if (id % 2 == 0) {
                  pthread_mutex_lock(&chopsticks[left]);
                  pthread_mutex_lock(&chopsticks[right]);
        } else {
                pthread_mutex_lock(&chopsticks[right]);
            pthread_mutex_lock(&chopsticks[left]);
        }

            printf("哲学家 %d 正在进餐\n", id);
        sleep(1);

        pthread_mutex_unlock(&chopsticks[right]);
        pthread_mutex_unlock(&chopsticks[left]);
    }
    return NULL;
}


int main() {
    pthread_t threads[N];
    int ids[N];

    // 初始化筷子互斥量
    for (int i = 0; i < N; i++) {
        pthread_mutex_init(&chopsticks[i], NULL);
    }

    // 创建哲学家线程
    for (int i = 0; i < N; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, philosopher, &ids[i]);
    }

    // 等待线程结束
    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }

    // 销毁互斥量
    for (int i = 0; i < N; i++) {
        pthread_mutex_destroy(&chopsticks[i]);
    }

    printf("所有哲学家任务完成，无死锁发生\n");
    return 0;
}