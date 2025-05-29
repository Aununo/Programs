#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h> // 用于获取文件大小
#include <fcntl.h>    // 用于文件操作
#include <netinet/in.h>
#include <pthread.h> // 用于线程
#include <errno.h>   // 用于 errno

#define BUFFER_SIZE 4096     // 缓冲区大小
#define PORT 8000            // Web服务器常用端口
#define WEB_ROOT "./webroot" // 存放网页和图片的根目录

// 函数：发送HTTP响应头
void send_http_response_header(int client_sock, const char *status, const char *content_type, long content_length) {
    char header_buffer[BUFFER_SIZE];
    // 确保有足够的空间，或者分多次发送
    snprintf(header_buffer, BUFFER_SIZE, "%s\r\n", status);
    write(client_sock, header_buffer, strlen(header_buffer));

    snprintf(header_buffer, BUFFER_SIZE, "Content-Type: %s\r\n", content_type);
    write(client_sock, header_buffer, strlen(header_buffer));

    if (content_length >= 0) {
        snprintf(header_buffer, BUFFER_SIZE, "Content-Length: %ld\r\n", content_length);
        write(client_sock, header_buffer, strlen(header_buffer));
    }

    snprintf(header_buffer, BUFFER_SIZE, "Connection: close\r\n"); // 短连接，处理完一个请求就关闭
    write(client_sock, header_buffer, strlen(header_buffer));

    write(client_sock, "\r\n", 2); // HTTP头结束的空行
}

// 函数：发送文件内容
void send_file(int client_sock, const char *file_path) {
    int file_fd;
    struct stat file_stat;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    // 检查文件是否存在且不是目录
    if (stat(file_path, &file_stat) == -1) {
        perror("stat error"); // 日志记录具体的 stat 错误
        if (errno == ENOENT) { // 文件不存在
             send_http_response_header(client_sock, "HTTP/1.1 404 Not Found", "text/html", -1);
             const char *msg_404 = "<html><body><h1>404 Not Found</h1><p>The requested file could not be found.</p></body></html>";
             write(client_sock, msg_404, strlen(msg_404));
        } else { // 其他 stat 错误 (例如权限问题)
             send_http_response_header(client_sock, "HTTP/1.1 500 Internal Server Error", "text/html", -1);
             const char *msg_500 = "<html><body><h1>500 Internal Server Error</h1><p>Could not access file information.</p></body></html>";
             write(client_sock, msg_500, strlen(msg_500));
        }
        return;
    }

    if (S_ISDIR(file_stat.st_mode)) { // 请求的是一个目录
        send_http_response_header(client_sock, "HTTP/1.1 403 Forbidden", "text/html", -1);
        const char *msg_403_dir = "<html><body><h1>403 Forbidden</h1><p>Directory listing is not allowed.</p></body></html>";
        write(client_sock, msg_403_dir, strlen(msg_403_dir));
        return;
    }


    file_fd = open(file_path, O_RDONLY);
    if (file_fd == -1) {
        perror("open error"); // 日志记录具体的 open 错误
        send_http_response_header(client_sock, "HTTP/1.1 500 Internal Server Error", "text/html", -1);
        const char *msg_500_open = "<html><body><h1>500 Internal Server Error</h1><p>Could not open the requested file.</p></body></html>";
        write(client_sock, msg_500_open, strlen(msg_500_open));
        return;
    }

    // 判断Content-Type
    const char *content_type = "application/octet-stream"; // 默认类型
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
    // 可以添加更多MIME类型判断

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

// 线程处理函数参数结构体
struct thread_args {
    int client_sock;
    struct sockaddr_in client_addr; // 用于日志记录客户端信息
};

// 线程处理函数
void *handle_client_request(void *arg) {
    struct thread_args *args = (struct thread_args *)arg;
    int client_sock = args->client_sock;
    struct sockaddr_in client_addr = args->client_addr; // 复制一份用于本线程
    char request_buffer[BUFFER_SIZE];                   // 每个线程有自己的请求缓冲区
    ssize_t read_len;

    // 尽快释放主线程分配的参数内存
    free(args);
    args = NULL; // 防止野指针

    pthread_detach(pthread_self()); // 分离线程，主线程不需要join它，资源由线程结束时自动回收

    printf("线程 %lu 处理客户端 %s:%d\n",
           (unsigned long)pthread_self(),
           inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port));

    // 5. 接收HTTP请求
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
    // printf("线程 %lu 收到请求:\n%s\n", (unsigned long)pthread_self(), request_buffer); // 调试时可以打开

    // 6. 解析HTTP GET请求
    char method[16], uri[PATH_MAX], version[16]; // 使用 PATH_MAX 以获得更大的 URI 缓冲区
    // 期望格式: "GET /path/file HTTP/1.1\r\n..."
    if (sscanf(request_buffer, "%15s %4095s %15s", method, uri, version) == 3) { // 注意uri的长度限制
        if (strcmp(method, "GET") == 0) {
            char file_path[PATH_MAX + sizeof(WEB_ROOT) + 2]; // 为 WEB_ROOT 和 '/' 留足空间
            
            // 处理根目录请求
            if (strcmp(uri, "/") == 0 || strcmp(uri, "/index.html") == 0) {
                snprintf(file_path, sizeof(file_path), "%s/index.html", WEB_ROOT);
            } else {
                // 确保 URI 以 / 开头，防止 WEB_ROOT 连接问题
                if (uri[0] == '/') {
                    snprintf(file_path, sizeof(file_path), "%s%s", WEB_ROOT, uri);
                } else {
                    snprintf(file_path, sizeof(file_path), "%s/%s", WEB_ROOT, uri);
                }
            }

            // 简单的路径遍历防护
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
        } else { // 其他方法如 POST, PUT 等
            send_http_response_header(client_sock, "HTTP/1.1 501 Not Implemented", "text/html", -1);
            const char *msg_501 = "<html><body><h1>501 Not Implemented</h1><p>The server does not support the requested method.</p></body></html>";
            write(client_sock, msg_501, strlen(msg_501));
        }
    } else { // 请求格式不正确
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
    socklen_t client_addr_size; // 在循环外声明即可，每次 accept 会更新

    // 1. 创建套接字
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("socket() error");
        exit(EXIT_FAILURE);
    }

    // 设置SO_REUSEADDR选项，允许服务器快速重启 (可选，但推荐)
    int opt = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        // 可以选择关闭套接字并退出，或者继续
        // close(server_sock);
        // exit(EXIT_FAILURE);
    }

    // 2. 初始化服务器地址结构
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 接受任意IP地址的连接
    server_addr.sin_port = htons(PORT);

    // 3. 绑定地址和端口
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind() error");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    // 4. 监听连接请求
    if (listen(server_sock, SOMAXCONN) == -1) { // 使用 SOMAXCONN 作为 backlog
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
            // 严重错误，可以考虑休眠一小段时间再重试，或者记录错误并继续
            sleep(1); // 避免在malloc持续失败时CPU空转
            continue;
        }

        client_addr_size = sizeof(args->client_addr); // accept前设置好大小
        args->client_sock = accept(server_sock, (struct sockaddr*)&(args->client_addr), &client_addr_size);

        if (args->client_sock == -1) {
            if (errno == EINTR) { // accept 被信号中断，常见，继续
                free(args);
                continue;
            }
            perror("accept() error");
            free(args);
            // 根据错误类型决定是否继续，如果是EMFILE或ENFILE等资源错误，可能需要停止接受新连接一段时间
            continue;
        }

        printf("客户端 %s:%d 已连接，准备创建线程处理...\n",
               inet_ntoa(args->client_addr.sin_addr),
               ntohs(args->client_addr.sin_port));

        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client_request, (void *)args) != 0) {
            perror("pthread_create() error");
            close(args->client_sock); // 关闭已接受的连接
            free(args);               // 释放为线程参数分配的内存
            // 线程创建失败是较严重的问题，可以记录日志
        }
        // 主线程不等待子线程，因为子线程已设置为 detached
        // args 的内存在 handle_client_request 中被 free
    }

    close(server_sock); // 理论上不会执行到这里，除非服务器被外部信号正常终止
    return 0;
}