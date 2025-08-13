#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define SERVER_PORT 8888
#define SERVER_IP "127.0.0.1"

int main()
{
    char buf[512];

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(0 > sockfd){
        perror("socket create error");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    int ret = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(0 > ret){
        perror("socket connect error");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("服务器连接成功... \n\n");
    for( ; ; ){
        memset(buf, 0x0, sizeof(buf));

        printf("Please enter a string : ");
        fgets(buf, sizeof(buf), stdin);

        ret = send(sockfd, buf,strlen(buf), 0);
        if(0 > ret){
            perror("send error");
            break;
        }

        if(0 == strncmp(buf, "exit", 4))
            break;
    }
    close(sockfd);
    return 0;
}