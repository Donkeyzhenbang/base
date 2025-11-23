// main_test.c
#include "input_simulator.h"
#include <unistd.h>

// 全局evdev设备引用
static struct evdev *global_evdev = NULL;

// 查找evdev设备 - 简化实现
struct evdev* find_evdev_by_name(const char *name)
{
    return global_evdev;
}

int main(void)
{
    printf("=== Linux Input Subsystem Simulator ===\n\n");
    
    // 初始化随机数种子
    srand(time(NULL));
    
    // 1. 初始化Input子系统
    printf("=== Phase 1: Initializing Input Subsystem ===\n");
    evdev_init();
    joydev_init();
    keyboard_init();
    
    // 等待设备注册完成
    sleep(1);
    
    printf("\n=== Phase 2: Creating Virtual Devices ===\n");
    
    // 创建虚拟evdev设备
    global_evdev = malloc(sizeof(struct evdev));
    if (!global_evdev) {
        printf("ERROR: Failed to allocate global_evdev\n");
        return -1;
    }
    
    snprintf(global_evdev->name, sizeof(global_evdev->name), "event0");
    INIT_LIST_HEAD(&global_evdev->client_list);
    pthread_mutex_init(&global_evdev->lock, NULL);
    global_evdev->minor = 0;
    
    printf("Created virtual evdev device: %s\n", global_evdev->name);
    
    printf("\n=== Phase 3: Simulating User Space Applications ===\n");
    
    // 模拟应用程序打开设备
    evdev_add_client(global_evdev, 100); // 应用1
    evdev_add_client(global_evdev, 101); // 应用2
    evdev_add_client(global_evdev, 102); // 应用3
    
    // 运行用户空间应用程序
    run_user_applications();
    
    // 等待用户应用完成
    sleep(2);
    
    printf("\n=== Phase 4: Starting Device Simulation ===\n");
    keyboard_start_simulation();
    
    // 等待键盘模拟完成
    printf("Waiting for keyboard simulation to complete...\n");
    sleep(8);  // 等待键盘线程完成（10个事件 × 0.6秒 ≈ 6秒）
    
    printf("\n=== Phase 5: Cleanup ===\n");
    
    // 模拟应用程序关闭设备
    evdev_remove_client(global_evdev, 100);
    evdev_remove_client(global_evdev, 101);
    evdev_remove_client(global_evdev, 102);
    
    // 清理全局evdev
    pthread_mutex_destroy(&global_evdev->lock);
    free(global_evdev);
    global_evdev = NULL;
    
    keyboard_exit();
    joydev_exit();
    evdev_exit();
    
    printf("\n=== Simulation Complete ===\n");
    return 0;
// main_test.c
#include "input_simulator.h"
#include <unistd.h>

// 全局evdev设备引用
static struct evdev *global_evdev = NULL;

// 查找evdev设备 - 简化实现
struct evdev* find_evdev_by_name(const char *name)
{
    return global_evdev;
}

int main(void)
{
    printf("=== Linux Input Subsystem Simulator ===\n\n");
    
    // 初始化随机数种子
    srand(time(NULL));
    
    // 1. 初始化Input子系统
    printf("=== Phase 1: Initializing Input Subsystem ===\n");
    evdev_init();
    joydev_init();
    keyboard_init();
    
    // 等待设备注册完成
    sleep(1);
    
    printf("\n=== Phase 2: Creating Virtual Devices ===\n");
    
    // 创建虚拟evdev设备
    global_evdev = malloc(sizeof(struct evdev));
    if (!global_evdev) {
        printf("ERROR: Failed to allocate global_evdev\n");
        return -1;
    }
    
    snprintf(global_evdev->name, sizeof(global_evdev->name), "event0");
    INIT_LIST_HEAD(&global_evdev->client_list);
    pthread_mutex_init(&global_evdev->lock, NULL);
    global_evdev->minor = 0;
    
    printf("Created virtual evdev device: %s\n", global_evdev->name);
    
    printf("\n=== Phase 3: Simulating User Space Applications ===\n");
    
    // 模拟应用程序打开设备
    evdev_add_client(global_evdev, 100); // 应用1
    evdev_add_client(global_evdev, 101); // 应用2
    evdev_add_client(global_evdev, 102); // 应用3
    
    // 运行用户空间应用程序
    run_user_applications();
    
    // 等待用户应用完成
    sleep(2);
    
    printf("\n=== Phase 4: Starting Device Simulation ===\n");
    keyboard_start_simulation();
    
    // 等待键盘模拟完成
    printf("Waiting for keyboard simulation to complete...\n");
    sleep(8);  // 等待键盘线程完成（10个事件 × 0.6秒 ≈ 6秒）
    
    printf("\n=== Phase 5: Cleanup ===\n");
    
    // 模拟应用程序关闭设备
    evdev_remove_client(global_evdev, 100);
    evdev_remove_client(global_evdev, 101);
    evdev_remove_client(global_evdev, 102);
    
    // 清理全局evdev
    pthread_mutex_destroy(&global_evdev->lock);
    free(global_evdev);
    global_evdev = NULL;
    
    keyboard_exit();
    joydev_exit();
    evdev_exit();
    
    printf("\n=== Simulation Complete ===\n");
    return 0;
}}
