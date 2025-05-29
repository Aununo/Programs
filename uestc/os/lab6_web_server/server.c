#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h> // 包含 struct sockaddr_in, AF_INET, htons 等

#define BUFFER_SIZE 1024
#define PORT 8080 // 可以自定义端口号

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size;
    char buffer[BUFFER_SIZE];
    int read_len;

    // 1. 创建套接字 (socket)
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("socket() error");
        exit(1);
    }

    // 2. 初始化服务器地址结构 (bind)
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 接受任意IP地址的连接
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind() error");
        close(server_sock);
        exit(1);
    }

    // 3. 监听连接请求 (listen)
    if (listen(server_sock, 5) == -1) { // 允许最多5个连接排队
        perror("listen() error");
        close(server_sock);
        exit(1);
    }

    printf("服务器启动，等待客户端连接在端口 %d...\n", PORT);

    // 4. 接受客户端连接 (accept)
    client_addr_size = sizeof(client_addr);
    client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_size);
    if (client_sock == -1) {
        perror("accept() error");
        close(server_sock);
        exit(1);
    }
    printf("客户端 %s:%d 已连接\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // 5. 与客户端通信 (read/write 或 recv/send)
    while (1) {
        // 从客户端接收消息
        read_len = read(client_sock, buffer, BUFFER_SIZE - 1);
        if (read_len <= 0) { // 客户端断开或出错
            if (read_len == 0) printf("客户端断开连接。\n");
            else perror("read() error from client");
            break;
        }
        buffer[read_len] = '\0';
        printf("收到客户端消息: %s", buffer); // 假设客户端发送的消息包含换行符

        if (strncmp(buffer, "exit\n", 5) == 0) {
            printf("收到客户端退出指令，服务器即将关闭。\n");
            break;
        }

        // 向客户端发送消息
        printf("回复客户端: ");
        fgets(buffer, BUFFER_SIZE, stdin); // 从标准输入读取服务器的回复
        write(client_sock, buffer, strlen(buffer));
        if (strncmp(buffer, "exit\n", 5) == 0) {
            printf("服务器主动退出。\n");
            break;
        }
    }

    // 6. 关闭套接字 (close)
    close(client_sock);
    close(server_sock);
    return 0;
}