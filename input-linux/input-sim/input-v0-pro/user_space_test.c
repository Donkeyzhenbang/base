// user_space_test.c
#include "input_simulator.h"

// 模拟用户空间应用程序
void user_application(const char *app_name)
{
    printf("USER[%s]: Starting application\n", app_name);
    
    // 模拟从设备读取事件
    for (int i = 0; i < 5; i++) {
        printf("USER[%s]: Waiting for input events...\n", app_name);
        sleep(1);
        
        // 在实际系统中，这里会调用read()从/dev/input/eventX读取事件
        // 我们这里只是模拟
        if (rand() % 3 == 0) {
            printf("USER[%s]: Received key event - processing...\n", app_name);
        }
    }
    
    printf("USER[%s]: Application finished\n", app_name);
}

// 模拟多个应用程序同时运行
void run_user_applications(void)
{
    printf("=== Starting User Space Applications ===\n");
    
    // 模拟多个应用程序 - 使用进程而不是线程避免竞争
    for (int i = 0; i < 3; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // 子进程
            const char *app_names[] = {"Text Editor", "Game", "Terminal"};
            user_application(app_names[i]);
            exit(0);
        } else if (pid < 0) {
            printf("Failed to fork process\n");
        }
    }
    
    // 等待子进程结束
    for (int i = 0; i < 3; i++) {
        int status;
        wait(&status);
    }
}
