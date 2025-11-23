// input_core.c
#include "input_simulator.h"

// 全局链表
static LIST_HEAD(input_dev_list);
static LIST_HEAD(input_handler_list);

// 注册input设备
int input_register_device(struct input_dev *dev)
{
    printf("INPUT_CORE: Registering device '%s'\n", dev->name);
    
    // 初始化handler链表
    INIT_LIST_HEAD(&dev->h_list);
    
    // 添加到全局设备链表
    list_add(&dev->node, &input_dev_list);
    
    // 与所有handler进行匹配
    struct input_handler *handler;
    struct list_head *pos;
    list_for_each(pos, &input_handler_list) {
        handler = list_entry(pos, struct input_handler, node);
        if (handler->connect) {
            handler->connect(handler, dev);
        }
    }
    
    return 0;
}

// 注销input设备
void input_unregister_device(struct input_dev *dev)
{
    printf("INPUT_CORE: Unregistering device '%s'\n", dev->name);
    list_del(&dev->node);
}

// 注册input handler
int input_register_handler(struct input_handler *handler)
{
    printf("INPUT_CORE: Registering handler '%s'\n", handler->name);
    
    // 初始化handler的h_list
    INIT_LIST_HEAD(&handler->h_list);
    
    // 添加到全局handler链表
    list_add(&handler->node, &input_handler_list);
    
    // 与所有设备进行匹配
    struct input_dev *dev;
    struct list_head *pos;
    list_for_each(pos, &input_dev_list) {
        dev = list_entry(pos, struct input_dev, node);
        if (handler->connect) {
            handler->connect(handler, dev);
        }
    }
    
    return 0;
}

// 注销input handler
void input_unregister_handler(struct input_handler *handler)
{
    printf("INPUT_CORE: Unregistering handler '%s'\n", handler->name);
    list_del(&handler->node);
}

// 上报输入事件
void input_event(struct input_dev *dev, 
                 unsigned int type, unsigned int code, int value)
{
    printf("INPUT_CORE: Device '%s' reporting event: type=0x%x, code=0x%x, value=%d\n",
           dev->name, type, code, value);
    
    // 遍历所有绑定的handler - 使用安全的遍历方式
    struct input_handle *handle;
    struct list_head *pos, *tmp;
    
    list_for_each_safe(pos, tmp, &dev->h_list) {
        handle = list_entry(pos, struct input_handle, d_node);
        
        // 添加空指针检查
        if (handle && handle->handler && handle->handler->event) {
            printf("INPUT_CORE: Calling handler '%s' for device '%s'\n", 
                   handle->handler->name, dev->name);
            handle->handler->event(handle, type, code, value);
        } else {
            printf("INPUT_CORE: WARNING: Invalid handle or handler for device '%s'\n", dev->name);
        }
    }
}

// 同步事件
void input_sync(struct input_dev *dev)
{
    input_event(dev, EV_SYN, SYN_REPORT, 0);
}

// 上报按键事件
void input_report_key(struct input_dev *dev, unsigned int code, int value)
{
    input_event(dev, EV_KEY, code, value);
}
