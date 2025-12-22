#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <stdatomic.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080

// --- 配置区域 ---
#define THREAD_COUNT 50     // 模拟并发线程数 (并发连接数)
#define REQUESTS_PER_THREAD 20000 // 每个线程发送多少个包
#define MSG_SIZE 64        // 每个包的大小 (字节)
// ----------------

char message[MSG_SIZE];
atomic_long total_requests = 0;
atomic_long total_bytes = 0;
atomic_int completed_threads = 0;

void *worker_thread(void *arg) {
    int sock;
    struct sockaddr_in serv_addr;
    char buffer[1024];

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return NULL;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sock);
        return NULL;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        close(sock);
        return NULL;
    }

    for (int i = 0; i < REQUESTS_PER_THREAD; i++) {
        if (send(sock, message, MSG_SIZE, 0) != MSG_SIZE) {
            perror("Send failed");
            break;
        }

        int valread = read(sock, buffer, 1024);
        if (valread <= 0) {
            perror("Read failed or server closed");
            break;
        }
        
        atomic_fetch_add(&total_requests, 1);
        atomic_fetch_add(&total_bytes, valread);
    }

    close(sock);
    atomic_fetch_add(&completed_threads, 1);
    return NULL;
}

long get_time_diff_ms(struct timeval start, struct timeval end) {
    return (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
}

int main() {
    pthread_t threads[THREAD_COUNT];
    struct timeval start, end;
    
    // 初始化测试数据
    memset(message, 'A', MSG_SIZE);

    printf("Starting Benchmark...\n");
    printf("Threads: %d, Requests/Thread: %d, Payload: %d bytes\n", 
            THREAD_COUNT, REQUESTS_PER_THREAD, MSG_SIZE);
    printf("Expected Total Requests: %d\n", THREAD_COUNT * REQUESTS_PER_THREAD);

    gettimeofday(&start, NULL);

    for (int i = 0; i < THREAD_COUNT; i++) {
        if (pthread_create(&threads[i], NULL, worker_thread, NULL) != 0) {
            perror("Failed to create thread");
        }
    }

    // 等待所有线程完成
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    gettimeofday(&end, NULL);

    long duration_ms = get_time_diff_ms(start, end);
    long reqs = atomic_load(&total_requests);
    long bytes = atomic_load(&total_bytes);

    double qps = (reqs * 1000.0) / duration_ms;
    double throughput_mb = (bytes / 1024.0 / 1024.0) / (duration_ms / 1000.0);

    printf("\n--- Results ---\n");
    printf("Time taken: %ld ms\n", duration_ms);
    printf("Total Requests: %ld\n", reqs);
    printf("QPS: %.2f\n", qps);
    printf("Throughput: %.2f MB/s\n", throughput_mb);

    return 0;
}