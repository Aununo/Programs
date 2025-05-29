#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h> // 包含 struct sockaddr_in, AF_INET, htons 等

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int read_len;

    if (argc != 3) {
        fprintf(stderr, "用法: %s <服务器IP> <服务器端口>\n", argv[0]);
        exit(1);
    }

    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    // 1. 创建套接字 (socket)
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket() error");
        exit(1);
    }

    // 2. 初始化服务器地址结构
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(server_port);

    // 3. 连接服务器 (connect)
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect() error");
        close(sock);
        exit(1);
    }
    printf("已连接到服务器 %s:%d\n", server_ip, server_port);

    // 4. 与服务器通信 (read/write 或 recv/send)
    while (1) {
        // 向服务器发送消息
        printf("发送给服务器: ");
        fgets(buffer, BUFFER_SIZE, stdin); // 从标准输入读取要发送的消息
        write(sock, buffer, strlen(buffer));

        if (strncmp(buffer, "exit\n", 5) == 0) {
            printf("客户端退出。\n");
            break;
        }

        // 从服务器接收消息
        read_len = read(sock, buffer, BUFFER_SIZE - 1);
        if (read_len <= 0) { // 服务器断开或出错
             if (read_len == 0) printf("服务器断开连接。\n");
             else perror("read() error from server");
            break;
        }
        buffer[read_len] = '\0';
        printf("收到服务器消息: %s", buffer); // 假设服务器发送的消息包含换行符
        if (strncmp(buffer, "exit\n", 5) == 0) {
            printf("收到服务器退出指令，客户端即将关闭。\n");
            break;
        }
    }

    // 5. 关闭套接字 (close)
    close(sock);
    return 0;
}