// keyboard_driver.c
#include "input_simulator.h"
#include <pthread.h>

static struct input_dev keyboard_dev;
static int keyboard_thread_running = 0;
static pthread_mutex_t keyboard_mutex = PTHREAD_MUTEX_INITIALIZER;

// 模拟按键的线程函数
static void* keyboard_thread(void *arg)
{
    printf("KEYBOARD: Starting keyboard simulation thread\n");
    
    // 模拟按键序列
    int keys[] = {KEY_A, KEY_S, KEY_D, KEY_F, KEY_G};
    int num_keys = sizeof(keys) / sizeof(keys[0]);
    
    for (int i = 0; i < 10; i++) {
        // 随机选择一个按键
        int key_index = rand() % num_keys;
        int key = keys[key_index];
        
        // 上报按键按下
        printf("KEYBOARD: Key PRESSED: %d\n", key);
        input_report_key(&keyboard_dev, key, 1);
        input_sync(&keyboard_dev);
        
        usleep(100000); // 100ms
        
        // 上报按键释放
        printf("KEYBOARD: Key RELEASED: %d\n", key);
        input_report_key(&keyboard_dev, key, 0);
        input_sync(&keyboard_dev);
        
        usleep(500000); // 500ms
    }
    
    pthread_mutex_lock(&keyboard_mutex);
    keyboard_thread_running = 0;
    pthread_mutex_unlock(&keyboard_mutex);
    
    printf("KEYBOARD: Simulation thread finished\n");
    return NULL;
}

// 初始化键盘设备
void keyboard_init(void)
{
    // 初始化设备
    strcpy(keyboard_dev.name, "simulated-keyboard");
    INIT_LIST_HEAD(&keyboard_dev.node);
    INIT_LIST_HEAD(&keyboard_dev.h_list);
    
    // 设置设备能力 - 修复移位警告
    keyboard_dev.evbit[0] = (1UL << EV_KEY) | (1UL << EV_REP);
    
    // 清零所有keybit
    for (int i = 0; i < 8; i++) {
        keyboard_dev.keybit[i] = 0;
    }
    
    // 设置支持的按键 - 使用UL后缀确保足够宽度
    keyboard_dev.keybit[0] = (1UL << KEY_A) | (1UL << KEY_S) | (1UL << KEY_D) | 
                            (1UL << KEY_F) | (1UL << KEY_G);
    
    input_register_device(&keyboard_dev);
    printf("KEYBOARD: Device initialized\n");
}

// 启动键盘模拟
void keyboard_start_simulation(void)
{
    pthread_mutex_lock(&keyboard_mutex);
    if (!keyboard_thread_running) {
        keyboard_thread_running = 1;
        pthread_t thread;
        if (pthread_create(&thread, NULL, keyboard_thread, NULL) == 0) {
            pthread_detach(thread);
            printf("KEYBOARD: Simulation thread created\n");
        } else {
            keyboard_thread_running = 0;
            printf("KEYBOARD: Failed to create simulation thread\n");
        }
    } else {
        printf("KEYBOARD: Simulation thread already running\n");
    }
    pthread_mutex_unlock(&keyboard_mutex);
}

// 退出键盘设备
void keyboard_exit(void)
{
    input_unregister_device(&keyboard_dev);
    printf("KEYBOARD: Device exited\n");
}
