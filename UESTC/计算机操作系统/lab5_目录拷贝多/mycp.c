#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <libgen.h>

#define BUFFER_SIZE 4096

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "mycp用法错误: mycp <源文件> <目标目录>\n");
        exit(1);
    }

    const char *src_path = argv[1];
    const char *dest_dir = argv[2];
    char final_dest_path[1024];

    char *src_filename = basename((char *)src_path);
    snprintf(final_dest_path, sizeof(final_dest_path), "%s/%s", dest_dir, src_filename);

    int src_fd = open(src_path, O_RDONLY);
    if (src_fd < 0) {
        perror("mycp: 打开源文件失败");
        exit(1);
    }

    int dest_fd = open(final_dest_path, O_WRONLY | O_CREAT | O_TRUNC, 0664);
    if (dest_fd < 0) {
        perror("mycp: 创建或打开目标文件失败");
        close(src_fd);
        exit(1);
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
        if (write(dest_fd, buffer, bytes_read) != bytes_read) {
            perror("mycp: 写入文件失败");
            break;
        }
    }

    if (bytes_read < 0) {
        perror("mycp: 读取文件失败");
    }

    close(src_fd);
    close(dest_fd);

    return 0;
}