#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <fcntl.h>  
#include <sys/stat.h>  
#include <mqueue.h>  
#include <errno.h>  
#include <unistd.h>  
#include <time.h>
 
#define QUEUE_NAME  "/mesg_p"  
#define MAX_SIZE    1024  
 
// 打印时分秒的宏        
#define PRINT_MIN_SEC do { \
            time_t t = time(NULL); \
            struct tm *tm_ptr = localtime(&t); \
            printf("%02d:%02d:%02d:", tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec);\
        } while (0);printf
 
int main(int argc, char *argv[]) 
{  
    mqd_t mqdes;  
    char buf[MAX_SIZE] = {0};  
    struct mq_attr attr;  
 
    // 命令行参数 
    // 第一个参数      S表示发送 R表示接收 D表示删除
    if (argc != 2) 
    {
        printf("Usage: %s S|R|D \n", argv[0]);
        return 0;
    }
    
    // 设置消息队列属性  
    attr.mq_flags = 0;  
    attr.mq_maxmsg = 10;  
    attr.mq_msgsize = MAX_SIZE;  
    attr.mq_curmsgs = 0;  
  
    // 打开或创建消息队列  
    mqdes = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0644, &attr);  
    if (mqdes == (mqd_t) -1) 
    {  
        perror("mq_open");  
        exit(1);  
    }  
 
    if (!strcmp(argv[1], "S"))
    {
        // 发送消息  
        strcpy(buf, "Mesg 12345678!");  
        if (mq_send(mqdes, buf, strlen(buf) + 1, 0) == -1) 
        {  
            perror("mq_send");  
            exit(1);  
        } 
        PRINT_MIN_SEC("Send: %s\n", buf); 
    } 
    else if (!strcmp(argv[1], "R")) 
    {
        // 接收消息  
        memset(buf, 0, sizeof(buf));  
        if (mq_receive(mqdes, buf, MAX_SIZE, NULL) == -1) 
        {  
            perror("mq_receive");  
            exit(1);  
        }  
        
        PRINT_MIN_SEC("Received: %s\n", buf); 
    } 
    else if (!strcmp(argv[1], "D")) 
    {
        // 删除消息队列  
        if (mq_unlink(QUEUE_NAME) == -1) 
        {  
            perror("mq_unlink");  
            exit(1);  
        } 
    } 
    else
    {
        printf("Usage: %s S|R|D \r\n", argv[0]);
        return 0;
    }
 
    // 关闭消息队列  
    if (mq_close(mqdes) == -1) 
    {  
        perror("mq_close");  
        exit(1);  
    }  
  
    return 0;  
}