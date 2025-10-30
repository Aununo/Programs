#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>


#define BUFFER_SIZE 4096
#define PORT 8000
#define WEB_ROOT "./webroot"

void send_http_response_header(int client_sock, const char *status, const char *content_type, long content_length) {
    char header_buffer[BUFFER_SIZE];
    sprintf(header_buffer, "%s\r\n", status);
    write(client_sock, header_buffer, strlen(header_buffer));
    sprintf(header_buffer, "Content-Type: %s\r\n", content_type);
    write(client_sock, header_buffer, strlen(header_buffer));
    if (content_length >= 0) {
        sprintf(header_buffer, "Content-Length: %ld\r\n", content_length);
        write(client_sock, header_buffer, strlen(header_buffer));
    }
    sprintf(header_buffer, "Connection: close\r\n");
    write(client_sock, header_buffer, strlen(header_buffer));
    write(client_sock, "\r\n", 2);
}

void send_file(int client_sock, const char *file_path) {
    int file_fd;
    struct stat file_stat;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    if (stat(file_path, &file_stat) == -1 || S_ISDIR(file_stat.st_mode)) {
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

    const char *content_type = "application/octet-stream";
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

    send_http_response_header(client_sock, "HTTP/1.1 200 OK", content_type, file_stat.st_size);

    while ((bytes_read = read(file_fd, buffer, BUFFER_SIZE)) > 0) {
        if (write(client_sock, buffer, bytes_read) != bytes_read) {
            perror("write error to client during file send");
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

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("socket() error");
        exit(1);
    }

    int opt = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
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

    if (listen(server_sock, 10) == -1) {
        perror("listen() error");
        close(server_sock);
        exit(1);
    }
    printf("Web服务器启动，监听端口 %d，Web根目录: %s\n", PORT, WEB_ROOT);

    while(1) {
        client_addr_size = sizeof(client_addr);
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_size);
        if (client_sock == -1) {
            perror("accept() error");
            continue;
        }
        printf("客户端 %s:%d 已连接\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        read_len = read(client_sock, request_buffer, BUFFER_SIZE - 1);
        if (read_len <= 0) {
            close(client_sock);
            continue;
        }
        request_buffer[read_len] = '\0';
        printf("收到HTTP请求:\n%s\n", request_buffer);

        char method[16], uri[256], version[16];
        if (sscanf(request_buffer, "%15s %255s %15s", method, uri, version) == 3) {
            if (strcmp(method, "GET") == 0) {
                char file_path[512];
                if (strcmp(uri, "/") == 0) {
                    sprintf(file_path, "%s/index.html", WEB_ROOT);
                } else {
                    sprintf(file_path, "%s%s", WEB_ROOT, uri);
                }
                if (strstr(file_path, "..")) {
                     send_http_response_header(client_sock, "HTTP/1.1 400 Bad Request", "text/html", -1);
                     const char *msg_400 = "<html><body><h1>400 Bad Request (Invalid Path)</h1></body></html>";
                     write(client_sock, msg_400, strlen(msg_400));
                } else {
                    printf("请求文件: %s\n", file_path);
                    send_file(client_sock, file_path);
                }
            } else {
                send_http_response_header(client_sock, "HTTP/1.1 501 Not Implemented", "text/html", -1);
                const char *msg_501 = "<html><body><h1>501 Not Implemented</h1></body></html>";
                write(client_sock, msg_501, strlen(msg_501));
            }
        } else {
            send_http_response_header(client_sock, "HTTP/1.1 400 Bad Request", "text/html", -1);
            const char *msg_400 = "<html><body><h1>400 Bad Request</h1></body></html>";
            write(client_sock, msg_400, strlen(msg_400));
        }

        close(client_sock);
        printf("客户端 %s:%d 连接已关闭\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }

    close(server_sock);
    return 0;
}