#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_PHILOSOPHERS 5 // 哲学家的数量

// 为五支筷子定义五个互斥量
pthread_mutex_t chopsticks[NUM_PHILOSOPHERS];

// 哲学家线程函数
void* philosopher(void* num) {
    int id = *(int*)num;
    int left_chopstick = id;
    int right_chopstick = (id + 1) % NUM_PHILOSOPHERS;

    while (1) {
        // 哲学家在思考
        printf("哲学家 %d 正在思考。\n", id);
        sleep(rand() % 3 + 1);

        // 循环尝试拿起筷子，直到成功
        while (1) {
            // 尝试拿起左边的筷子（非阻塞）
            // 对应实验指导中的 pthread_mutex_trylock 操作
            if (pthread_mutex_trylock(&chopsticks[left_chopstick]) == 0) {
                printf("哲学家 %d 拿起了左边的筷子 %d。\n", id, left_chopstick);
                
                // 成功拿到左筷子后，尝试拿右筷子（非阻塞）
                if (pthread_mutex_trylock(&chopsticks[right_chopstick]) == 0) {
                    printf("哲学家 %d 拿起了右边的筷子 %d。\n", id, right_chopstick);
                    // 两只都拿到了，跳出循环准备进餐
                    break; 
                } else {
                    // 如果拿不到右筷子，则立即释放已拿到的左筷子，实现“让权等待”
                    printf("哲学家 %d 拿不到右筷子 %d, 准备放下左筷子 %d。\n", id, right_chopstick, left_chopstick);
                    pthread_mutex_unlock(&chopsticks[left_chopstick]);
                }
            }
            
            // 只要没成功拿到两只筷子，就等待一个随机的小段时间再试，以打破活锁循环
            // 产生一个 100ms 到 200ms 之间的随机延时
            usleep(rand() % 100000 + 100000);
        }

        // 进餐
        printf("哲学家 %d 正在进餐。\n", id);
        sleep(rand() % 3 + 1);

        // 放下筷子
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

    // 等待所有哲学家线程
    for (i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_join(philosophers[i], NULL);
    }

    // 销毁互斥量
    for (i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_mutex_destroy(&chopsticks[i]);
    }

    return 0;
}