#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>

#define BUFFER_SIZE 4096
#define PORT 8000
#define WEB_ROOT "./webroot"

void send_http_response_header(int client_sock, const char *status, const char *content_type, long content_length) {
    char header_buffer[BUFFER_SIZE];
    snprintf(header_buffer, BUFFER_SIZE, "%s\r\n", status);
    write(client_sock, header_buffer, strlen(header_buffer));

    snprintf(header_buffer, BUFFER_SIZE, "Content-Type: %s\r\n", content_type);
    write(client_sock, header_buffer, strlen(header_buffer));

    if (content_length >= 0) {
        snprintf(header_buffer, BUFFER_SIZE, "Content-Length: %ld\r\n", content_length);
        write(client_sock, header_buffer, strlen(header_buffer));
    }

    snprintf(header_buffer, BUFFER_SIZE, "Connection: close\r\n");
    write(client_sock, header_buffer, strlen(header_buffer));

    write(client_sock, "\r\n", 2);
}

void send_file(int client_sock, const char *file_path) {
    int file_fd;
    struct stat file_stat;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    if (stat(file_path, &file_stat) == -1) {
        perror("stat error");
        if (errno == ENOENT) {
             send_http_response_header(client_sock, "HTTP/1.1 404 Not Found", "text/html", -1);
             const char *msg_404 = "<html><body><h1>404 Not Found</h1><p>The requested file could not be found.</p></body></html>";
             write(client_sock, msg_404, strlen(msg_404));
        } else {
             send_http_response_header(client_sock, "HTTP/1.1 500 Internal Server Error", "text/html", -1);
             const char *msg_500 = "<html><body><h1>500 Internal Server Error</h1><p>Could not access file information.</p></body></html>";
             write(client_sock, msg_500, strlen(msg_500));
        }
        return;
    }

    if (S_ISDIR(file_stat.st_mode)) {
        send_http_response_header(client_sock, "HTTP/1.1 403 Forbidden", "text/html", -1);
        const char *msg_403_dir = "<html><body><h1>403 Forbidden</h1><p>Directory listing is not allowed.</p></body></html>";
        write(client_sock, msg_403_dir, strlen(msg_403_dir));
        return;
    }


    file_fd = open(file_path, O_RDONLY);
    if (file_fd == -1) {
        perror("open error");
        send_http_response_header(client_sock, "HTTP/1.1 500 Internal Server Error", "text/html", -1);
        const char *msg_500_open = "<html><body><h1>500 Internal Server Error</h1><p>Could not open the requested file.</p></body></html>";
        write(client_sock, msg_500_open, strlen(msg_500_open));
        return;
    }

    const char *content_type = "application/octet-stream";
    if (strstr(file_path, ".html") || strstr(file_path, ".htm")) {
        content_type = "text/html; charset=utf-8";
    } else if (strstr(file_path, ".jpg") || strstr(file_path, ".jpeg")) {
        content_type = "image/jpeg";
    } else if (strstr(file_path, ".png")) {
        content_type = "image/png";
    } else if (strstr(file_path, ".gif")) {
        content_type = "image/gif";
    } else if (strstr(file_path, ".css")) {
        content_type = "text/css";
    } else if (strstr(file_path, ".js")) {
        content_type = "application/javascript";
    } else if (strstr(file_path, ".txt")) {
        content_type = "text/plain; charset=utf-8";
    }

    send_http_response_header(client_sock, "HTTP/1.1 200 OK", content_type, file_stat.st_size);

    while ((bytes_read = read(file_fd, buffer, BUFFER_SIZE)) > 0) {
        if (write(client_sock, buffer, bytes_read) != bytes_read) {
            perror("write error to client during file send");
            break;
        }
    }
    if (bytes_read < 0) {
        perror("read error from file during send");
    }
    close(file_fd);
}

struct thread_args {
    int client_sock;
    struct sockaddr_in client_addr;
};

void *handle_client_request(void *arg) {
    struct thread_args *args = (struct thread_args *)arg;
    int client_sock = args->client_sock;
    struct sockaddr_in client_addr = args->client_addr;
    char request_buffer[BUFFER_SIZE];
    ssize_t read_len;

    free(args);
    args = NULL;

    pthread_detach(pthread_self());

    printf("线程 %lu 处理客户端 %s:%d\n",
           (unsigned long)pthread_self(),
           inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port));

    read_len = read(client_sock, request_buffer, BUFFER_SIZE - 1);
    if (read_len <= 0) {
        if (read_len < 0) {
            perror("read() error from client in thread");
        } else {
            fprintf(stderr, "线程 %lu: 客户端 %s:%d 过早关闭连接。\n",
                    (unsigned long)pthread_self(),
                    inet_ntoa(client_addr.sin_addr),
                    ntohs(client_addr.sin_port));
        }
        close(client_sock);
        pthread_exit(NULL);
    }
    request_buffer[read_len] = '\0';

    char method[16], uri[PATH_MAX], version[16];
    if (sscanf(request_buffer, "%15s %4095s %15s", method, uri, version) == 3) {
        if (strcmp(method, "GET") == 0) {
            char file_path[PATH_MAX + sizeof(WEB_ROOT) + 2];
            
            if (strcmp(uri, "/") == 0 || strcmp(uri, "/index.html") == 0) {
                snprintf(file_path, sizeof(file_path), "%s/index.html", WEB_ROOT);
            } else {
                if (uri[0] == '/') {
                    snprintf(file_path, sizeof(file_path), "%s%s", WEB_ROOT, uri);
                } else {
                    snprintf(file_path, sizeof(file_path), "%s/%s", WEB_ROOT, uri);
                }
            }

            if (strstr(file_path, "..")) {
                 send_http_response_header(client_sock, "HTTP/1.1 400 Bad Request", "text/html", -1);
                 const char *msg_400 = "<html><body><h1>400 Bad Request</h1><p>Invalid path specified.</p></body></html>";
                 write(client_sock, msg_400, strlen(msg_400));
            } else {
                printf("线程 %lu: 客户端 %s:%d 请求文件: %s\n",
                       (unsigned long)pthread_self(),
                       inet_ntoa(client_addr.sin_addr),
                       ntohs(client_addr.sin_port),
                       file_path);
                send_file(client_sock, file_path);
            }
        } else {
            send_http_response_header(client_sock, "HTTP/1.1 501 Not Implemented", "text/html", -1);
            const char *msg_501 = "<html><body><h1>501 Not Implemented</h1><p>The server does not support the requested method.</p></body></html>";
            write(client_sock, msg_501, strlen(msg_501));
        }
    } else {
        send_http_response_header(client_sock, "HTTP/1.1 400 Bad Request", "text/html", -1);
        const char *msg_400_format = "<html><body><h1>400 Bad Request</h1><p>Your browser sent a request that this server could not understand.</p></body></html>";
        write(client_sock, msg_400_format, strlen(msg_400_format));
    }

    close(client_sock);
    printf("线程 %lu 处理完毕，客户端 %s:%d 连接已关闭\n",
           (unsigned long)pthread_self(),
           inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port));
    pthread_exit(NULL);
}


int main() {
    int server_sock;
    struct sockaddr_in server_addr;
    socklen_t client_addr_size;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("socket() error");
        exit(EXIT_FAILURE);
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
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, SOMAXCONN) == -1) {
        perror("listen() error");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("并发Web服务器(线程模型)启动，监听端口 %d，Web根目录: %s\n", PORT, WEB_ROOT);
    printf("请在浏览器中访问: http://localhost:%d\n", PORT);


    while(1) {
        struct thread_args *args = malloc(sizeof(struct thread_args));
        if (!args) {
            perror("malloc for thread_args failed");
            sleep(1);
            continue;
        }

        client_addr_size = sizeof(args->client_addr);
        args->client_sock = accept(server_sock, (struct sockaddr*)&(args->client_addr), &client_addr_size);

        if (args->client_sock == -1) {
            if (errno == EINTR) {
                free(args);
                continue;
            }
            perror("accept() error");
            free(args);
            continue;
        }

        printf("客户端 %s:%d 已连接，准备创建线程处理...\n",
               inet_ntoa(args->client_addr.sin_addr),
               ntohs(args->client_addr.sin_port));

        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client_request, (void *)args) != 0) {
            perror("pthread_create() error");
            close(args->client_sock);
            free(args);
        }
    }

    close(server_sock);
    return 0;
}