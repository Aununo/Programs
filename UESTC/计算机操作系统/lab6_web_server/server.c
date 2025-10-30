#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024
#define PORT 8080

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size;
    char buffer[BUFFER_SIZE];
    int read_len;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("socket() error");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind() error");
        close(server_sock);
        exit(1);
    }

    if (listen(server_sock, 5) == -1) {
        perror("listen() error");
        close(server_sock);
        exit(1);
    }

    printf("服务器启动，等待客户端连接在端口 %d...\n", PORT);

    client_addr_size = sizeof(client_addr);
    client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_size);
    if (client_sock == -1) {
        perror("accept() error");
        close(server_sock);
        exit(1);
    }
    printf("客户端 %s:%d 已连接\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    while (1) {
        read_len = read(client_sock, buffer, BUFFER_SIZE - 1);
        if (read_len <= 0) {
            if (read_len == 0) printf("客户端断开连接。\n");
            else perror("read() error from client");
            break;
        }
        buffer[read_len] = '\0';
        printf("收到客户端消息: %s", buffer);

        if (strncmp(buffer, "exit\n", 5) == 0) {
            printf("收到客户端退出指令，服务器即将关闭。\n");
            break;
        }

        printf("回复客户端: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        write(client_sock, buffer, strlen(buffer));
        if (strncmp(buffer, "exit\n", 5) == 0) {
            printf("服务器主动退出。\n");
            break;
        }
    }

    close(client_sock);
    close(server_sock);
    return 0;
}