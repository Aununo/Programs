#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h> // for basename()

#define BUFFER_SIZE 4096 // 缓冲区大小

// 函数声明
void copy_file(const char *src_path, const char *dest_path);
void copy_directory(const char *src_path, const char *dest_path);

/**
 * @brief 主函数，程序入口
 * @param argc 命令行参数数量 
 * @param argv 命令行参数数组 
 * @return 成功返回0，失败返回1
 */
int main(int argc, char *argv[]) {
    // 检查命令行参数数量是否正确 
    if (argc != 3) {
        fprintf(stderr, "用法: %s <源路径> <目标路径>\n", argv[0]);
        exit(1);
    }

    const char *src_path = argv[1];
    const char *dest_path = argv[2];

    struct stat src_stat;
    // 使用stat获取源路径信息，lstat可以处理符号链接，但本实验用stat即可
    if (stat(src_path, &src_stat) != 0) {
        perror("读取源路径信息失败");
        exit(1);
    }

    // 判断源路径是文件还是目录
    if (S_ISDIR(src_stat.st_mode)) {
        // 源是目录，执行目录拷贝
        copy_directory(src_path, dest_path);
    } else if (S_ISREG(src_stat.st_mode)) {
        // 源是文件
        struct stat dest_stat;
        // 检查目标路径是否存在及其类型
        if (stat(dest_path, &dest_stat) == 0 && S_ISDIR(dest_stat.st_mode)) {
            // 目标是目录，将源文件拷贝到该目录下
            char new_dest_path[1024];
            // 从源路径中提取文件名
            char *src_basename = basename((char *)src_path);
            snprintf(new_dest_path, sizeof(new_dest_path), "%s/%s", dest_path, src_basename);
            copy_file(src_path, new_dest_path);
        } else {
            // 目标是文件或不存在，执行文件到文件的拷贝
            copy_file(src_path, dest_path);
        }
    } else {
        fprintf(stderr, "不支持的源文件类型\n");
        exit(1);
    }

    printf("拷贝完成。\n");
    return 0;
}

/**
 * @brief 拷贝单个文件
 * @param src_path 源文件路径
 * @param dest_path 目标文件路径
 */
void copy_file(const char *src_path, const char *dest_path) {
    int src_fd, dest_fd;
    ssize_t bytes_read;
    char buffer[BUFFER_SIZE];
    struct stat src_stat;

    // 打开源文件（只读） 
    src_fd = open(src_path, O_RDONLY);
    if (src_fd < 0) {
        perror("打开源文件失败");
        return;
    }
    
    // 获取源文件的权限信息
    if (fstat(src_fd, &src_stat) < 0) {
        perror("获取源文件权限失败");
        close(src_fd);
        return;
    }

    // 创建或打开目标文件（只写，创建时赋予和源文件相同的权限，如果存在则清空） 
    dest_fd = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, src_stat.st_mode);
    if (dest_fd < 0) {
        perror("创建或打开目标文件失败");
        close(src_fd);
        return;
    }

    // 循环读取源文件并写入目标文件，直到文件末尾 
    while ((bytes_read = read(src_fd, buffer, BUFFER_SIZE)) > 0) {
        if (write(dest_fd, buffer, bytes_read) != bytes_read) {
            perror("写入目标文件失败");
            break;
        }
    }

    if (bytes_read < 0) {
        perror("读取源文件失败");
    }

    // 关闭文件描述符 
    close(src_fd);
    close(dest_fd);
}

/**
 * @brief 递归拷贝目录
 * @param src_path 源目录路径
 * @param dest_path 目标目录路径
 */
void copy_directory(const char *src_path, const char *dest_path) {
    struct stat dest_stat;
    // 检查目标路径
    if (stat(dest_path, &dest_stat) != 0) {
        // 目标不存在，创建它
        if (mkdir(dest_path, 0755) != 0) { // 默认权限0755
            perror("创建目标目录失败");
            return;
        }
    } else if (!S_ISDIR(dest_stat.st_mode)) {
        // 目标存在但不是目录，报错
        fprintf(stderr, "错误: 目标路径 %s 已存在但不是一个目录。\n", dest_path);
        return;
    }

    DIR *dir = opendir(src_path);
    if (!dir) {
        perror("打开源目录失败");
        return;
    }

    struct dirent *entry;
    // 遍历源目录中的所有条目
    while ((entry = readdir(dir)) != NULL) {
        // 忽略 . 和 .. 目录
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
        
        // 判断条目类型并递归处理
        if (S_ISDIR(entry_stat.st_mode)) {
            copy_directory(new_src_path, new_dest_path); // 递归拷贝子目录
        } else {
            copy_file(new_src_path, new_dest_path); // 拷贝文件
        }
    }

    closedir(dir);
}