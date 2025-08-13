#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define SERVER_PORT 8888

int main() 
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(0 > sockfd){
        perror("socket create error");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    int ret = bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(0 > ret){
        perror("bind error");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    //服务器进入监听状态
    ret = listen(sockfd, 50);
    if(0 > ret){
        perror("listen error");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    //阻塞等待客户端连接
    struct sockaddr_in client_addr;
    char ip_str[20];
    char recv_buf[512];
    int addrlen = sizeof(client_addr);
    int connfd = accept(sockfd, (struct sockaddr*)&client_addr, &addrlen);

    printf("有客户端接入... \n");
    inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip_str, sizeof(ip_str));
    printf("客户端主机IP地址：%s \n", ip_str);
    printf("客户端主机端口号： %d \n",client_addr.sin_port);
    
    for( ; ; ){
        memset(recv_buf, 0x0, sizeof(recv_buf));
        ret = recv(connfd, recv_buf, sizeof(recv_buf), 0);
        if(0 >= ret){
            perror("recv error");
            close(connfd);
            break;
        }

        printf("from client : %s \n", recv_buf);
        if(0 == strncmp("exit", recv_buf, 4)){
            printf("server exit ! \n");
            close(connfd);
            break;
        }

    }
    close(sockfd);
    return 0;
}