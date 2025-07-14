/*
 * test_app.c - 用于测试 proc_lister 驱动的用户空间程序
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>   // 包含 open 函数 [cite: 137]
#include <unistd.h>  // 包含 read, write, close 函数 [cite: 138, 139, 140]
#include <sys/ioctl.h> // 包含 ioctl 函数 [cite: 141]

#define DEVICE_PATH "/dev/proc_lister" // 设备文件路径 [cite: 120]

int main() {
    int fd;
    char dummy_buffer[16];

    printf("启动测试程序...\n");
    printf("将通过设备文件 '%s' 与驱动交互。\n\n", DEVICE_PATH);

    // 1. 打开设备文件，这将触发驱动的 my_open 方法
    printf("1. 正在调用 open()...\n");
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("错误：打开设备文件失败");
        return -1;
    }
    printf("   > open() 调用成功。请使用 'dmesg' 命令查看内核日志。\n\n");

    // 2. 读取设备文件，这将触发驱动的 my_read 方法
    printf("2. 正在调用 read()...\n");
    read(fd, dummy_buffer, sizeof(dummy_buffer));
    printf("   > read() 调用完成。请使用 'dmesg' 命令查看内核日志。\n\n");

    // 3. 写入设备文件，这将触发驱动的 my_write 方法
    printf("3. 正在调用 write()...\n");
    write(fd, "test", 4);
    printf("   > write() 调用完成。请使用 'dmesg' 命令查看内核日志。\n\n");

    // 4. 对设备文件进行IO控制，这将触发驱动的 my_ioctl 方法
    printf("4. 正在调用 ioctl()...\n");
    ioctl(fd, 0, NULL); // cmd 和 arg 参数在这里是虚拟的
    printf("   > ioctl() 调用完成。请使用 'dmesg' 命令查看内核日志。\n\n");

    // 5. 关闭设备文件，这将触发驱动的 my_release 方法
    printf("5. 正在调用 close()...\n");
    close(fd);
    printf("   > close() 调用成功。请使用 'dmesg' 命令查看内核日志。\n\n");

    printf("测试程序执行完毕。\n");

    return 0;
}