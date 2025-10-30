#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

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

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket() error");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(server_port);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect() error");
        close(sock);
        exit(1);
    }
    printf("已连接到服务器 %s:%d\n", server_ip, server_port);

    while (1) {
        printf("发送给服务器: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        write(sock, buffer, strlen(buffer));

        if (strncmp(buffer, "exit\n", 5) == 0) {
            printf("客户端退出。\n");
            break;
        }

        read_len = read(sock, buffer, BUFFER_SIZE - 1);
        if (read_len <= 0) {
             if (read_len == 0) printf("服务器断开连接。\n");
             else perror("read() error from server");
            break;
        }
        buffer[read_len] = '\0';
        printf("收到服务器消息: %s", buffer);
        if (strncmp(buffer, "exit\n", 5) == 0) {
            printf("收到服务器退出指令，客户端即将关闭。\n");
            break;
        }
    }

    close(sock);
    return 0;
}