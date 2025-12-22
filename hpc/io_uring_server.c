#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <liburing.h>

#define PORT 8080
#define QUEUE_DEPTH 256
#define BUF_SIZE 1024

// 定义请求类型
enum {
    EVENT_ACCEPT,
    EVENT_READ,
    EVENT_WRITE
};

// 用于在 io_uring 中传递上下文信息的结构体
typedef struct {
    int fd;
    int event_type;
    char buffer[BUF_SIZE];
    struct iovec iov;
} Request;

struct io_uring ring;

// 辅助函数：添加 Accept 请求
void add_accept_request(int server_socket, struct sockaddr_in *client_addr, socklen_t *client_addr_len) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
    Request *req = malloc(sizeof(Request));
    req->event_type = EVENT_ACCEPT;
    req->fd = server_socket;

    io_uring_prep_accept(sqe, server_socket, (struct sockaddr *)client_addr, client_addr_len, 0);
    io_uring_sqe_set_data(sqe, req);
}

// 辅助函数：添加 Read 请求
void add_read_request(int client_socket) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
    Request *req = malloc(sizeof(Request));
    req->event_type = EVENT_READ;
    req->fd = client_socket;
    
    // 设置 iovec 用于接收数据
    req->iov.iov_base = req->buffer;
    req->iov.iov_len = BUF_SIZE;

    io_uring_prep_readv(sqe, client_socket, &req->iov, 1, 0);
    io_uring_sqe_set_data(sqe, req);
}

// 辅助函数：添加 Write 请求 (回显数据)
void add_write_request(int client_socket, char *data, size_t len) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
    Request *req = malloc(sizeof(Request));
    req->event_type = EVENT_WRITE;
    req->fd = client_socket;

    // 复制数据到写缓冲区
    memcpy(req->buffer, data, len);
    req->iov.iov_base = req->buffer;
    req->iov.iov_len = len;

    io_uring_prep_writev(sqe, client_socket, &req->iov, 1, 0);
    io_uring_sqe_set_data(sqe, req);
}

int main() {
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    int server_socket;

    // 1. 初始化 io_uring
    if (io_uring_queue_init(QUEUE_DEPTH, &ring, 0) < 0) {
        perror("io_uring_queue_init");
        return 1;
    }

    // 2. 创建监听 Socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        return 1;
    }

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    if (listen(server_socket, 10) < 0) {
        perror("Listen failed");
        return 1;
    }

    printf("Server listening on port %d using io_uring...\n", PORT);

    // 3. 提交第一个 Accept 请求
    add_accept_request(server_socket, &client_addr, &client_len);
    io_uring_submit(&ring);

    // 4. 事件循环
    struct io_uring_cqe *cqe;
    while (1) {
        int ret = io_uring_wait_cqe(&ring, &cqe);
        if (ret < 0) {
            perror("io_uring_wait_cqe");
            break;
        }

        Request *req = (Request *)io_uring_cqe_get_data(cqe);
        
        // 检查操作结果 (res 字段)
        if (cqe->res < 0) {
            fprintf(stderr, "Async request failed: %s\n", strerror(-cqe->res));
        }

        switch (req->event_type) {
            case EVENT_ACCEPT: {
                int client_fd = cqe->res;
                if (client_fd >= 0) {
                    printf("New connection: FD %d\n", client_fd);
                    // 为新连接添加读请求
                    add_read_request(client_fd);
                }
                // 重新添加 Accept 请求以接受下一个连接
                add_accept_request(server_socket, &client_addr, &client_len);
                free(req); // Accept 的 req 不需要保持，可以释放
                break;
            }
            case EVENT_READ: {
                int bytes_read = cqe->res;
                if (bytes_read <= 0) {
                    // 连接关闭或出错
                    printf("Connection closed: FD %d\n", req->fd);
                    close(req->fd);
                    free(req);
                } else {
                    // 读到了数据，添加写请求 (回显)
                    // 注意：这里为了演示简单，创建了新的 Write Request
                    // 实际高性能场景可以复用结构体
                    add_write_request(req->fd, req->buffer, bytes_read);
                    free(req); // 读请求完成，释放资源
                }
                break;
            }
            case EVENT_WRITE: {
                // 写操作完成
                // 继续监听该 FD 的读事件
                add_read_request(req->fd);
                free(req); // 写请求完成，释放资源
                break;
            }
        }

        // 标记 CQE 已处理
        io_uring_cqe_seen(&ring, cqe);
        
        // 提交所有新生成的 SQE
        io_uring_submit(&ring);
    }

    io_uring_queue_exit(&ring);
    close(server_socket);
    return 0;
}

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <fcntl.h>
// #include <sys/socket.h>
// #include <sys/resource.h>
// #include <netinet/in.h>
// #include <netinet/tcp.h>
// #include <liburing.h>

// #define PORT 8080
// #define MAX_CONNECTIONS 65536  // 最大连接数，需小于 ulimit -n
// #define BACKLOG 4096
// #define RING_DEPTH 8192        // io_uring 队列深度
// #define BUF_SIZE 1024

// // 定义操作类型
// enum {
//     OP_ACCEPT,
//     OP_READ,
//     OP_WRITE
// };

// // 连接上下文，预分配在全局数组中
// typedef struct {
//     int fd;
//     int op;
//     char buffer[BUF_SIZE];
//     struct iovec iov;
// } ConnContext;

// // 预分配所有可能的连接上下文
// // 简单的做法是直接用 FD 作为数组下标
// ConnContext conns[MAX_CONNECTIONS];

// struct io_uring ring;

// // 设置非阻塞模式
// void set_nonblocking(int fd) {
//     int flags = fcntl(fd, F_GETFL, 0);
//     fcntl(fd, F_SETFL, flags | O_NONBLOCK);
// }

// // 提升文件描述符限制
// void setup_rlimit() {
//     struct rlimit limit;
//     if (getrlimit(RLIMIT_NOFILE, &limit) < 0) {
//         perror("getrlimit");
//         exit(1);
//     }
//     // 尝试提升到最大值
//     limit.rlim_cur = MAX_CONNECTIONS;
//     if (limit.rlim_cur > limit.rlim_max) limit.rlim_cur = limit.rlim_max;
    
//     if (setrlimit(RLIMIT_NOFILE, &limit) < 0) {
//         perror("setrlimit failed (try running with sudo or ulimit -n 65536)");
//         exit(1);
//     }
//     printf("Max open files set to: %lu\n", limit.rlim_cur);
// }

// // 提交 Accept 请求
// void add_accept(int server_fd) {
//     struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
//     ConnContext *ctx = &conns[server_fd]; // 监听 socket 的上下文
//     ctx->fd = server_fd;
//     ctx->op = OP_ACCEPT;

//     // accept 的第三、四个参数由内核填充，这里简单起见传 NULL
//     // 如果需要客户端 IP，需传入全局的 sockaddr 缓存
//     io_uring_prep_accept(sqe, server_fd, NULL, NULL, 0);
//     io_uring_sqe_set_data(sqe, ctx);
// }

// // 提交 Read 请求
// void add_read(int fd) {
//     struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
//     ConnContext *ctx = &conns[fd];
//     ctx->fd = fd;
//     ctx->op = OP_READ;
//     ctx->iov.iov_base = ctx->buffer;
//     ctx->iov.iov_len = BUF_SIZE;

//     io_uring_prep_readv(sqe, fd, &ctx->iov, 1, 0);
//     io_uring_sqe_set_data(sqe, ctx);
// }

// // 提交 Write 请求
// void add_write(int fd, int bytes) {
//     struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
//     ConnContext *ctx = &conns[fd];
//     ctx->fd = fd;
//     ctx->op = OP_WRITE;
//     ctx->iov.iov_len = bytes;

//     io_uring_prep_writev(sqe, fd, &ctx->iov, 1, 0);
//     io_uring_sqe_set_data(sqe, ctx);
// }

// int main() {
//     setup_rlimit();

//     // 1. 初始化 io_uring
//     struct io_uring_params params;
//     memset(&params, 0, sizeof(params));
    
//     // 如果内核支持 IORING_SETUP_SINGLE_ISSUER，可以启用以提升性能（可选）
//     // params.flags |= IORING_SETUP_SINGLE_ISSUER;

//     if (io_uring_queue_init_params(RING_DEPTH, &ring, &params) < 0) {
//         perror("io_uring_queue_init");
//         return 1;
//     }

//     // 2. 创建监听 Socket
//     int server_fd = socket(AF_INET, SOCK_STREAM, 0);
//     if (server_fd < 0) {
//         perror("Socket");
//         return 1;
//     }

//     int opt = 1;
//     setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
//     setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)); // 支持多进程/线程绑定同一端口
    
//     // 禁用 Nagle 算法，降低延迟
//     setsockopt(server_fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));

//     struct sockaddr_in addr;
//     addr.sin_family = AF_INET;
//     addr.sin_addr.s_addr = INADDR_ANY;
//     addr.sin_port = htons(PORT);

//     if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
//         perror("Bind");
//         return 1;
//     }

//     if (listen(server_fd, BACKLOG) < 0) {
//         perror("Listen");
//         return 1;
//     }

//     // 3. 初始提交 Accept
//     add_accept(server_fd);

//     printf("High-Performance Server listening on %d. Ring depth: %d\n", PORT, RING_DEPTH);

//     // 4. 事件循环
//     struct io_uring_cqe *cqe;
//     while (1) {
//         // 提交所有待处理的 SQE，并等待至少 1 个 CQE 完成
//         int ret = io_uring_submit_and_wait(&ring, 1);
//         if (ret < 0) {
//             perror("submit_and_wait");
//             break;
//         }

//         // 批量处理所有已完成的事件（CQ 队列中的事件）
//         unsigned head;
//         unsigned count = 0;

//         io_uring_for_each_cqe(&ring, head, cqe) {
//             count++;
//             ConnContext *ctx = (ConnContext *)io_uring_cqe_get_data(cqe);
//             int res = cqe->res;

//             if (res < 0) {
//                 // 错误处理 (忽略 -ECANCELED 等)
//                 if (res != -ECANCELED && ctx->fd != server_fd) {
//                     close(ctx->fd);
//                 }
//             } else {
//                 switch (ctx->op) {
//                     case OP_ACCEPT:
//                         // res 是新的文件描述符
//                         if (res >= 0 && res < MAX_CONNECTIONS) {
//                             // 为新连接添加 READ
//                             // 实际生产中应检查是否超过 MAX_CONNECTIONS
//                             add_read(res); 
//                         } else {
//                             if (res >= 0) close(res); // 超过数组限制，关闭
//                         }
//                         // 继续监听新的连接
//                         add_accept(server_fd);
//                         break;

//                     case OP_READ:
//                         if (res == 0) {
//                             // 客户端断开
//                             close(ctx->fd);
//                         } else {
//                             // 读到数据，改为写回 (Echo)
//                             add_write(ctx->fd, res);
//                         }
//                         break;

//                     case OP_WRITE:
//                         // 写完数据，继续读
//                         add_read(ctx->fd);
//                         break;
//                 }
//             }
//         }
        
//         // 标记这批 CQE 已处理，推进内核队列指针
//         io_uring_cq_advance(&ring, count);
//     }

//     close(server_fd);
//     io_uring_queue_exit(&ring);
//     return 0;
// }