// evdev_handler.c
#include "input_simulator.h"

// 全局evdev设备链表
static LIST_HEAD(evdev_list);
static int evdev_minor_counter = 0;

// 向客户端传递事件
static void evdev_pass_event(struct evdev_client *client, 
                            struct input_event *event)
{
    // 添加到环形缓冲区
    client->buffer[client->head] = *event;
    client->head = (client->head + 1) % 64;
    
    // 如果缓冲区满，丢弃最旧的事件
    if (client->head == client->tail) {
        client->tail = (client->tail + 1) % 64;
        printf("EVDEV: Buffer overflow, dropping oldest event\n");
    }
    
    printf("EVDEV: Event passed to client fd=%d: type=0x%x, code=0x%x, value=%d\n",
           client->fd, event->type, event->code, event->value);
}

// evdev事件处理函数
static void evdev_event(struct input_handle *handle,
                       unsigned int type, unsigned int code, int value)
{
    if (!handle || !handle->private) {
        printf("EVDEV: ERROR: Invalid handle in evdev_event\n");
        return;
    }
    
    struct evdev *evdev = (struct evdev *)handle->private;
    struct input_event event;
    
    // 获取当前时间
    gettimeofday(&event.time, NULL);
    event.type = type;
    event.code = code;
    event.value = value;
    
    printf("EVDEV: Processing event for '%s'\n", evdev->name);
    
    // 加锁保护客户端链表
    pthread_mutex_lock(&evdev->lock);
    
    // 遍历所有客户端，传递事件
    struct evdev_client *client;
    struct list_head *pos, *tmp;
    list_for_each_safe(pos, tmp, &evdev->client_list) {
        client = list_entry(pos, struct evdev_client, node);
        evdev_pass_event(client, &event);
    }
    
    pthread_mutex_unlock(&evdev->lock);
}

// evdev连接函数
static int evdev_connect(struct input_handler *handler, struct input_dev *dev)
{
    if (!handler || !dev) {
        printf("EVDEV: ERROR: Invalid handler or device in connect\n");
        return -1;
    }
    
    struct evdev *evdev = malloc(sizeof(struct evdev));
    if (!evdev) {
        printf("EVDEV: ERROR: Failed to allocate evdev\n");
        return -1;
    }
    
    // 初始化evdev设备
    snprintf(evdev->name, sizeof(evdev->name), "event%d", evdev_minor_counter);
    evdev->minor = evdev_minor_counter++;
    INIT_LIST_HEAD(&evdev->client_list);
    pthread_mutex_init(&evdev->lock, NULL);
    
    // 设置input handle
    evdev->handle.dev = dev;
    evdev->handle.handler = handler;
    evdev->handle.private = evdev;
    INIT_LIST_HEAD(&evdev->handle.d_node);
    INIT_LIST_HEAD(&evdev->handle.h_node);
    
    // 添加到设备的handler列表
    list_add(&evdev->handle.d_node, &dev->h_list);
    
    // 添加到handler的h_list
    list_add(&evdev->handle.h_node, &handler->h_list);
    
    printf("EVDEV: Connected device '%s' to handler, created '%s'\n", 
           dev->name, evdev->name);
    
    return 0;
}

// evdev断开连接
static void evdev_disconnect(struct input_handle *handle)
{
    if (!handle || !handle->private) {
        printf("EVDEV: ERROR: Invalid handle in disconnect\n");
        return;
    }
    
    struct evdev *evdev = (struct evdev *)handle->private;
    printf("EVDEV: Disconnecting '%s'\n", evdev->name);
    
    // 清理所有客户端
    struct evdev_client *client, *tmp;
    pthread_mutex_lock(&evdev->lock);
    
    list_for_each_entry_safe(client, tmp, &evdev->client_list, node) {
        list_del(&client->node);
        free(client);
    }
    
    pthread_mutex_unlock(&evdev->lock);
    
    // 销毁互斥锁
    pthread_mutex_destroy(&evdev->lock);
    
    // 从链表中删除
    list_del(&handle->d_node);
    list_del(&handle->h_node);
    
    free(evdev);
}

// 添加客户端
void evdev_add_client(struct evdev *evdev, int fd)
{
    if (!evdev) {
        printf("EVDEV: ERROR: Invalid evdev in add_client\n");
        return;
    }
    
    struct evdev_client *client = malloc(sizeof(struct evdev_client));
    if (!client) {
        printf("EVDEV: ERROR: Failed to allocate client\n");
        return;
    }
    
    client->fd = fd;
    client->head = 0;
    client->tail = 0;
    INIT_LIST_HEAD(&client->node);
    
    // 加锁保护客户端链表
    pthread_mutex_lock(&evdev->lock);
    list_add(&client->node, &evdev->client_list);
    pthread_mutex_unlock(&evdev->lock);
    
    printf("EVDEV: Added client fd=%d to '%s'\n", fd, evdev->name);
}

// 移除客户端
void evdev_remove_client(struct evdev *evdev, int fd)
{
    if (!evdev) {
        printf("EVDEV: ERROR: Invalid evdev in remove_client\n");
        return;
    }
    
    struct evdev_client *client, *tmp;
    int found = 0;
    
    // 加锁保护客户端链表
    pthread_mutex_lock(&evdev->lock);
    
    list_for_each_entry_safe(client, tmp, &evdev->client_list, node) {
        if (client->fd == fd) {
            list_del(&client->node);
            free(client);
            found = 1;
            break;
        }
    }
    
    pthread_mutex_unlock(&evdev->lock);
    
    if (found) {
        printf("EVDEV: Removed client fd=%d from '%s'\n", fd, evdev->name);
    } else {
        printf("EVDEV: Client fd=%d not found in '%s'\n", fd, evdev->name);
    }
}

// evdev handler定义
static struct input_handler evdev_handler = {
    .name = "evdev",
    .event = evdev_event,
    .connect = evdev_connect,
    .disconnect = evdev_disconnect,
};

// 初始化evdev
void evdev_init(void)
{
    input_register_handler(&evdev_handler);
    printf("EVDEV: Handler initialized\n");
}

// 退出evdev
void evdev_exit(void)
{
    input_unregister_handler(&evdev_handler);
    printf("EVDEV: Handler exited\n");
}
