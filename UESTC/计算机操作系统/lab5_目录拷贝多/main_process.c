#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "用法: %s <源目录> <目标目录>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *src_dir_path = argv[1];
    const char *dest_dir_path = argv[2];
    struct stat dest_stat;

    if (stat(dest_dir_path, &dest_stat) == -1) {
        if (mkdir(dest_dir_path, 0755) == -1) {
            perror("创建目标目录失败");
            exit(EXIT_FAILURE);
        }
        printf("目标目录 '%s' 已创建。\n", dest_dir_path);
    } else if (!S_ISDIR(dest_stat.st_mode)) {
        fprintf(stderr, "错误: 目标路径 '%s' 已存在但不是一个目录。\n", dest_dir_path);
        exit(EXIT_FAILURE);
    }

    DIR *dir = opendir(src_dir_path);
    if (dir == NULL) {
        perror("打开源目录失败");
        exit(EXIT_FAILURE);
    }

    printf("开始遍历目录 '%s'...\n", src_dir_path);

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char full_src_path[1024];
        snprintf(full_src_path, sizeof(full_src_path), "%s/%s", src_dir_path, entry->d_name);

        struct stat entry_stat;
        if (stat(full_src_path, &entry_stat) == -1) {
            perror("获取文件属性失败");
            continue;
        }

        if (S_ISREG(entry_stat.st_mode)) {
            pid_t pid = fork();

            if (pid < 0) {
                perror("fork失败");
                continue;
            } else if (pid == 0) {
                printf("子进程(PID: %d)开始拷贝文件: %s\n", getpid(), full_src_path);
                execl("./mycp", "mycp", full_src_path, dest_dir_path, NULL);
                perror("execl执行失败");
                exit(1);
            } else {
                int status;
                waitpid(pid, &status, 0);
                if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                    printf("父进程: 子进程(PID: %d)已成功完成拷贝。\n", pid);
                } else {
                    fprintf(stderr, "父进程: 子进程(PID: %d)拷贝失败。\n", pid);
                }
            }
        }
    }

    closedir(dir);
    printf("所有文件拷贝任务已处理完毕。\n");
    return 0;
}