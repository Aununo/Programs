#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h> // 用于获取文件大小
#include <fcntl.h>    // 用于文件操作
#include <netinet/in.h>


#define BUFFER_SIZE 4096 // 增大缓冲区以处理HTTP头和文件内容
#define PORT 8000        // Web服务器常用端口
#define WEB_ROOT "./webroot" // 存放网页和图片的根目录，请先创建这个目录

// 函数：发送HTTP响应头
void send_http_response_header(int client_sock, const char *status, const char *content_type, long content_length) {
    char header_buffer[BUFFER_SIZE];
    sprintf(header_buffer, "%s\r\n", status);
    write(client_sock, header_buffer, strlen(header_buffer));
    sprintf(header_buffer, "Content-Type: %s\r\n", content_type);
    write(client_sock, header_buffer, strlen(header_buffer));
    if (content_length >= 0) { // 对于HEAD请求或错误页面，可能没有Content-Length
        sprintf(header_buffer, "Content-Length: %ld\r\n", content_length);
        write(client_sock, header_buffer, strlen(header_buffer));
    }
    sprintf(header_buffer, "Connection: close\r\n"); // 短连接，处理完一个请求就关闭
    write(client_sock, header_buffer, strlen(header_buffer));
    write(client_sock, "\r\n", 2); // HTTP头结束的空行
}

// 函数：发送文件内容
void send_file(int client_sock, const char *file_path) {
    int file_fd;
    struct stat file_stat;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    if (stat(file_path, &file_stat) == -1 || S_ISDIR(file_stat.st_mode)) { // 文件不存在或是目录
        send_http_response_header(client_sock, "HTTP/1.1 404 Not Found", "text/html", -1);
        const char *msg_404 = "<html><body><h1>404 Not Found</h1></body></html>";
        write(client_sock, msg_404, strlen(msg_404));
        return;
    }

    file_fd = open(file_path, O_RDONLY);
    if (file_fd == -1) {
        send_http_response_header(client_sock, "HTTP/1.1 500 Internal Server Error", "text/html", -1);
        const char *msg_500 = "<html><body><h1>500 Internal Server Error</h1></body></html>";
        write(client_sock, msg_500, strlen(msg_500));
        return;
    }

    // 判断Content-Type
    const char *content_type = "application/octet-stream"; // 默认类型
    if (strstr(file_path, ".html") || strstr(file_path, ".htm")) {
        content_type = "text/html";
    } else if (strstr(file_path, ".jpg") || strstr(file_path, ".jpeg")) {
        content_type = "image/jpeg";
    } else if (strstr(file_path, ".png")) {
        content_type = "image/png";
    } else if (strstr(file_path, ".css")) {
        content_type = "text/css";
    } else if (strstr(file_path, ".js")) {
        content_type = "application/javascript";
    }
    // 可以添加更多MIME类型判断

    send_http_response_header(client_sock, "HTTP/1.1 200 OK", content_type, file_stat.st_size);

    while ((bytes_read = read(file_fd, buffer, BUFFER_SIZE)) > 0) {
        if (write(client_sock, buffer, bytes_read) != bytes_read) {
            perror("write error to client during file send");
            // 发生写入错误，可能客户端已关闭连接，可以提前退出循环
            break;
        }
    }
    close(file_fd);
}


int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size;
    char request_buffer[BUFFER_SIZE];
    int read_len;

    // 1. 创建套接字
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("socket() error");
        exit(1);
    }

    // 设置SO_REUSEADDR选项，允许服务器快速重启
    int opt = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
    }


    // 2. 初始化服务器地址结构
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind() error");
        close(server_sock);
        exit(1);
    }

    // 3. 监听
    if (listen(server_sock, 10) == -1) { // 增加监听队列
        perror("listen() error");
        close(server_sock);
        exit(1);
    }
    printf("Web服务器启动，监听端口 %d，Web根目录: %s\n", PORT, WEB_ROOT);

    // 4. 循环接受客户端连接
    while(1) {
        client_addr_size = sizeof(client_addr);
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_size);
        if (client_sock == -1) {
            perror("accept() error");
            continue; // 继续等待下一个连接
        }
        printf("客户端 %s:%d 已连接\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // 5. 接收HTTP请求 (这里只做简单处理)
        read_len = read(client_sock, request_buffer, BUFFER_SIZE - 1);
        if (read_len <= 0) {
            close(client_sock);
            continue;
        }
        request_buffer[read_len] = '\0';
        printf("收到HTTP请求:\n%s\n", request_buffer);

        // 6. 解析HTTP GET请求 (非常简化的解析)
        char method[16], uri[256], version[16];
        // 期望格式: "GET /path/file HTTP/1.1\r\n..."
        if (sscanf(request_buffer, "%15s %255s %15s", method, uri, version) == 3) {
            if (strcmp(method, "GET") == 0) {
                char file_path[512];
                // 如果URI是"/"，则提供默认的index.html
                if (strcmp(uri, "/") == 0) {
                    sprintf(file_path, "%s/index.html", WEB_ROOT);
                } else {
                    sprintf(file_path, "%s%s", WEB_ROOT, uri);
                }
                // 防止路径遍历攻击 (简单示例，实际需要更健壮的检查)
                if (strstr(file_path, "..")) {
                     send_http_response_header(client_sock, "HTTP/1.1 400 Bad Request", "text/html", -1);
                     const char *msg_400 = "<html><body><h1>400 Bad Request (Invalid Path)</h1></body></html>";
                     write(client_sock, msg_400, strlen(msg_400));
                } else {
                    printf("请求文件: %s\n", file_path);
                    send_file(client_sock, file_path);
                }
            } else {
                // 处理其他方法或返回 501 Not Implemented
                send_http_response_header(client_sock, "HTTP/1.1 501 Not Implemented", "text/html", -1);
                const char *msg_501 = "<html><body><h1>501 Not Implemented</h1></body></html>";
                write(client_sock, msg_501, strlen(msg_501));
            }
        } else {
            // 无效请求
            send_http_response_header(client_sock, "HTTP/1.1 400 Bad Request", "text/html", -1);
            const char *msg_400 = "<html><body><h1>400 Bad Request</h1></body></html>";
            write(client_sock, msg_400, strlen(msg_400));
        }

        // 7. 关闭客户端套接字 (HTTP/1.0 或 Connection: close 的行为)
        close(client_sock);
        printf("客户端 %s:%d 连接已关闭\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }

    close(server_sock);
    return 0;
}