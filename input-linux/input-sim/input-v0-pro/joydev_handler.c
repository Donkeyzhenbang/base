// joydev_handler.c
#include "input_simulator.h"

// joydev事件处理函数
static void joydev_event(struct input_handle *handle,
                        unsigned int type, unsigned int code, int value)
{
    if (!handle || !handle->private) {
        printf("JOYDEV: ERROR: Invalid handle in joydev_event\n");
        return;
    }
    
    struct joydev *joydev = (struct joydev *)handle->private;
    
    printf("JOYDEV: '%s' processing event: type=0x%x, code=0x%x, value=%d\n",
           joydev->name, type, code, value);
    
    // 在实际实现中，这里会将事件转换为游戏杆协议并传递给用户空间
    // 这里我们只是打印日志
    if (type == EV_KEY) {
        printf("JOYDEV: Button event - code=%d, value=%d\n", code, value);
    } else if (type == EV_ABS) {
        printf("JOYDEV: Axis event - code=%d, value=%d\n", code, value);
    }
}

// joydev连接函数
static int joydev_connect(struct input_handler *handler, struct input_dev *dev)
{
    if (!handler || !dev) {
        printf("JOYDEV: ERROR: Invalid handler or device in connect\n");
        return -1;
    }
    
    struct joydev *joydev = malloc(sizeof(struct joydev));
    if (!joydev) {
        printf("JOYDEV: ERROR: Failed to allocate joydev\n");
        return -1;
    }
    
    // 初始化joydev设备
    snprintf(joydev->name, sizeof(joydev->name), "js%d", rand() % 10);
    joydev->handle.dev = dev;
    joydev->handle.handler = handler;
    joydev->handle.private = joydev;
    INIT_LIST_HEAD(&joydev->handle.d_node);
    INIT_LIST_HEAD(&joydev->handle.h_node);
    
    // 添加到链表
    list_add(&joydev->handle.d_node, &dev->h_list);
    list_add(&joydev->handle.h_node, &handler->h_list);
    
    printf("JOYDEV: Connected device '%s' to handler, created '%s'\n", 
           dev->name, joydev->name);
    
    return 0;
}

// joydev断开连接
static void joydev_disconnect(struct input_handle *handle)
{
    if (!handle || !handle->private) {
        printf("JOYDEV: ERROR: Invalid handle in disconnect\n");
        return;
    }
    
    struct joydev *joydev = (struct joydev *)handle->private;
    printf("JOYDEV: Disconnecting '%s'\n", joydev->name);
    
    list_del(&handle->d_node);
    list_del(&handle->h_node);
    free(joydev);
}

// joydev handler定义
static struct input_handler joydev_handler = {
    .name = "joydev",
    .event = joydev_event,
    .connect = joydev_connect,
    .disconnect = joydev_disconnect,
};

// 初始化joydev
void joydev_init(void)
{
    input_register_handler(&joydev_handler);
    printf("JOYDEV: Handler initialized\n");
}

// 退出joydev
void joydev_exit(void)
{
    input_unregister_handler(&joydev_handler);
    printf("JOYDEV: Handler exited\n");
}
