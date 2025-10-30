#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE_PATH "/dev/proc_lister"

int main() {
    int fd;
    char dummy_buffer[16];

    printf("启动测试程序...\n");
    printf("将通过设备文件 '%s' 与驱动交互。\n\n", DEVICE_PATH);

    printf("1. 正在调用 open()...\n");
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("错误：打开设备文件失败");
        return -1;
    }
    printf("   > open() 调用成功。请使用 'dmesg' 命令查看内核日志。\n\n");

    printf("2. 正在调用 read()...\n");
    read(fd, dummy_buffer, sizeof(dummy_buffer));
    printf("   > read() 调用完成。请使用 'dmesg' 命令查看内核日志。\n\n");

    printf("3. 正在调用 write()...\n");
    write(fd, "test", 4);
    printf("   > write() 调用完成。请使用 'dmesg' 命令查看内核日志。\n\n");

    printf("4. 正在调用 ioctl()...\n");
    ioctl(fd, 0, NULL);
    printf("   > ioctl() 调用完成。请使用 'dmesg' 命令查看内核日志。\n\n");

    printf("5. 正在调用 close()...\n");
    close(fd);
    printf("   > close() 调用成功。请使用 'dmesg' 命令查看内核日志。\n\n");

    printf("测试程序执行完毕。\n");

    return 0;
}