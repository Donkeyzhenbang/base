# Makefile for io_uring examples

CC = gcc
CFLAGS = -Wall -O2 -g -D_GNU_SOURCE
LDFLAGS = -luring -lpthread

# 默认目标
all: server client

# 服务器程序
server: io_uring_server.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# 客户端测试程序
client: io_uring_client.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# 检查系统是否支持io_uring
check_uring:
	@echo "Checking io_uring support..."
	@$(CC) -o /tmp/test_uring test_uring_simple.c -luring 2>/dev/null && \
		/tmp/test_uring && echo "io_uring is supported" || echo "io_uring is NOT supported"
	@rm -f /tmp/test_uring

# 测试程序
test_uring_simple.c:
	@echo '#include <stdio.h>' > test_uring_simple.c
	@echo '#include <liburing.h>' >> test_uring_simple.c
	@echo 'int main() {' >> test_uring_simple.c
	@echo '    struct io_uring ring;' >> test_uring_simple.c
	@echo '    int ret = io_uring_queue_init(8, &ring, 0);' >> test_uring_simple.c
	@echo '    if (ret < 0) {' >> test_uring_simple.c
	@echo '        fprintf(stderr, "io_uring not supported: %d\\n", ret);' >> test_uring_simple.c
	@echo '        return 1;' >> test_uring_simple.c
	@echo '    }' >> test_uring_simple.c
	@echo '    printf("io_uring initialized successfully!\\n");' >> test_uring_simple.c
	@echo '    io_uring_queue_exit(&ring);' >> test_uring_simple.c
	@echo '    return 0;' >> test_uring_simple.c
	@echo '}' >> test_uring_simple.c

# 清理
clean:
	rm -f server client *.o test_uring_simple.c

# 运行服务器
run-server: server
	./server

# 运行简单测试
run-test: client
	./client -c 5 -m 10

# 压力测试
run-stress: client
	./client -c 100 -m 1000

# 显示帮助
help:
	@echo "Available targets:"
	@echo "  all        - Build server and client (default)"
	@echo "  server     - Build the io_uring server"
	@echo "  client     - Build the test client"
	@echo "  check_uring - Check if system supports io_uring"
	@echo "  run-server - Run the server"
	@echo "  run-test   - Run a simple test (5 clients, 10 msgs each)"
	@echo "  run-stress - Run a stress test (100 clients, 1000 msgs each)"
	@echo "  clean      - Clean build files"

.PHONY: all clean run-server run-test run-stress help check_uring