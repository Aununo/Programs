#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/wait.h>

#define N 5
#define LEFT (id)
#define RIGHT ((id + 1) % N)

sem_t *chopsticks[N];

void philosopher(int id) {
    int left = LEFT;
    int right = RIGHT;
    for (int i = 0; i < 30; i++) {
        printf("哲学家 %d 正在思考\n", id);
        sleep(1);
        if (id % 2 == 0) {
            sem_wait(chopsticks[left]);  // 拿左筷子
            sem_wait(chopsticks[right]); // 拿右筷子
        } else {
            sem_wait(chopsticks[right]); // 拿右筷子
            sem_wait(chopsticks[left]);  // 拿左筷子
        }
        printf("哲学家 %d 正在进餐\n", id);
        sleep(1);
        sem_post(chopsticks[right]); // 放下右筷子
        sem_post(chopsticks[left]);  // 放下左筷子
    }
}

int main() {
    // 创建有名信号量
    for (int i = 0; i < N; i++) {
        char name[20];
        sprintf(name, "/chopstick%d", i);
        chopsticks[i] = sem_open(name, O_CREAT, 0644, 1);
        if (chopsticks[i] == SEM_FAILED) {
            perror("sem_open 失败");
            exit(1);
        }
    }

    // 创建子进程
    for (int i = 0; i < N; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            philosopher(i);
            exit(0);
        }
    }

    // 等待子进程结束
    for (int i = 0; i < N; i++) {
        wait(NULL);
    }

    // 关闭和删除信号量
    for (int i = 0; i < N; i++) {
        sem_close(chopsticks[i]);
        char name[20];
        sprintf(name, "/chopstick%d", i);
        sem_unlink(name);
    }

    printf("所有哲学家任务完成，无死锁发生\n");
    return 0;
}