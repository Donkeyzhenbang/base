#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

using namespace std;

// 协议处理器声明
void handle_http(int client_fd);
void handle_custom(int client_fd);

const int PORT = 8888;
const int BUFFER_SIZE = 1024;

int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void protocol_router(int client_fd) {
    char buf[BUFFER_SIZE];
    
    // 使用MSG_PEEK查看数据而不移除
    ssize_t bytes = recv(client_fd, buf, BUFFER_SIZE, MSG_PEEK);
    
    if(bytes > 0) {
        // HTTP协议识别
        if(strncmp(buf, "GET", 3) == 0 || strncmp(buf, "POST", 4) == 0) {
            handle_http(client_fd);
        }
        // 自定义协议识别（示例：以0x01开头）
        else if(buf[0] == 0x01) {
            handle_custom(client_fd);
        }
    }
}

void handle_http(int client_fd) {
    char response[] = "HTTP/1.1 200 OK\r\n"
                      "Content-Length: 12\r\n\r\n"
                      "Hello HTTP!";
    send(client_fd, response, sizeof(response), 0);
    close(client_fd);
}

void handle_custom(int client_fd) {
    char header;
    recv(client_fd, &header, 1, 0); // 读取协议头
    
    char response[] = {0x01, 0x02, 0x00, 0x0C, 'C','u','s','t','o','m',' ','O','K'};
    send(client_fd, response, sizeof(response), 0);
    close(client_fd);
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd == -1) {
        cerr << "Socket creation failed" << endl;
        return 1;
    }

    // 设置地址重用
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if(bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        cerr << "Bind failed" << endl;
        close(server_fd);
        return 1;
    }

    if(listen(server_fd, 5) < 0) {
        cerr << "Listen failed" << endl;
        close(server_fd);
        return 1;
    }

    // 设置为非阻塞模式
    set_nonblocking(server_fd);

    fd_set read_fds;
    vector<int> client_fds;
    int max_fd = server_fd;

    while(true) {
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);
        
        // 添加所有客户端socket
        for(int fd : client_fds) {
            FD_SET(fd, &read_fds);
            if(fd > max_fd) max_fd = fd;
        }

        // 设置500ms超时
        timeval timeout{.tv_sec = 0, .tv_usec = 500000};
        int activity = select(max_fd + 1, &read_fds, nullptr, nullptr, &timeout);

        if(activity < 0) {
            cerr << "Select error" << endl;
            break;
        }

        // 处理新连接
        if(FD_ISSET(server_fd, &read_fds)) {
            sockaddr_in client_addr{};
            socklen_t addr_len = sizeof(client_addr);
            int client_fd = accept(server_fd, (sockaddr*)&client_addr, &addr_len);
            
            if(client_fd > 0) {
                set_nonblocking(client_fd);
                client_fds.push_back(client_fd);
                cout << "New connection: " << client_fd << endl;
            }
        }

        // 处理客户端数据
        for(auto it = client_fds.begin(); it != client_fds.end();) {
            int fd = *it;
            if(FD_ISSET(fd, &read_fds)) {
                protocol_router(fd);
                it = client_fds.erase(it);
            } else {
                ++it;
            }
        }
    }

    // 清理资源
    close(server_fd);
    for(int fd : client_fds) close(fd);
    return 0;
}
