#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>

#define BUFFER_SIZE 4096

void copy_file(const char *src_path, const char *dest_path);
void copy_directory(const char *src_path, const char *dest_path);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "用法: %s <源路径> <目标路径>\n", argv[0]);
        exit(1);
    }

    const char *src_path = argv[1];
    const char *dest_path = argv[2];

    struct stat src_stat;
    if (stat(src_path, &src_stat) != 0) {
        perror("读取源路径信息失败");
        exit(1);
    }

    if (S_ISDIR(src_stat.st_mode)) {
        copy_directory(src_path, dest_path);
    } else if (S_ISREG(src_stat.st_mode)) {
        struct stat dest_stat;
        if (stat(dest_path, &dest_stat) == 0 && S_ISDIR(dest_stat.st_mode)) {
            char new_dest_path[1024];
            char *src_basename = basename((char *)src_path);
            snprintf(new_dest_path, sizeof(new_dest_path), "%s/%s", dest_path, src_basename);
            copy_file(src_path, new_dest_path);
        } else {
            copy_file(src_path, dest_path);
        }
    } else {
        fprintf(stderr, "不支持的源文件类型\n");
        exit(1);
    }

    printf("拷贝完成。\n");
    return 0;
}

void copy_file(const char *src_path, const char *dest_path) {
    int src_fd, dest_fd;
    ssize_t bytes_read;
    char buffer[BUFFER_SIZE];
    struct stat src_stat;

    src_fd = open(src_path, O_RDONLY);
    if (src_fd < 0) {
        perror("打开源文件失败");
        return;
    }
    
    if (fstat(src_fd, &src_stat) < 0) {
        perror("获取源文件权限失败");
        close(src_fd);
        return;
    }

    dest_fd = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, src_stat.st_mode);
    if (dest_fd < 0) {
        perror("创建或打开目标文件失败");
        close(src_fd);
        return;
    }

    while ((bytes_read = read(src_fd, buffer, BUFFER_SIZE)) > 0) {
        if (write(dest_fd, buffer, bytes_read) != bytes_read) {
            perror("写入目标文件失败");
            break;
        }
    }

    if (bytes_read < 0) {
        perror("读取源文件失败");
    }

    close(src_fd);
    close(dest_fd);
}

void copy_directory(const char *src_path, const char *dest_path) {
    struct stat dest_stat;
    if (stat(dest_path, &dest_stat) != 0) {
        if (mkdir(dest_path, 0755) != 0) {
            perror("创建目标目录失败");
            return;
        }
    } else if (!S_ISDIR(dest_stat.st_mode)) {
        fprintf(stderr, "错误: 目标路径 %s 已存在但不是一个目录。\n", dest_path);
        return;
    }

    DIR *dir = opendir(src_path);
    if (!dir) {
        perror("打开源目录失败");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char new_src_path[1024];
        char new_dest_path[1024];
        snprintf(new_src_path, sizeof(new_src_path), "%s/%s", src_path, entry->d_name);
        snprintf(new_dest_path, sizeof(new_dest_path), "%s/%s", dest_path, entry->d_name);

        struct stat entry_stat;
        if (stat(new_src_path, &entry_stat) != 0) {
            perror("读取条目信息失败");
            continue;
        }
        
        if (S_ISDIR(entry_stat.st_mode)) {
            copy_directory(new_src_path, new_dest_path);
        } else {
            copy_file(new_src_path, new_dest_path);
        }
    }

    closedir(dir);
}