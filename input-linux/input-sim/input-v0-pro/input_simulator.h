// input_simulator.h
#ifndef INPUT_SIMULATOR_H
#define INPUT_SIMULATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/wait.h>

// 模拟内核中的基本类型
typedef unsigned int __u32;
typedef unsigned short __u16;
typedef int __s32;

// Input事件定义 (与内核保持一致)
struct input_event {
    struct timeval time;
    __u16 type;
    __u16 code;
    __s32 value;
};

// 事件类型
#define EV_SYN          0x00
#define EV_KEY          0x01
#define EV_REL          0x02
#define EV_ABS          0x03
#define EV_MSC          0x04
#define EV_SW           0x05
#define EV_LED          0x11
#define EV_SND          0x12
#define EV_REP          0x14
#define EV_FF           0x15
#define EV_PWR          0x16
#define EV_FF_STATUS    0x17
#define EV_MAX          0x1f

// 同步事件
#define SYN_REPORT      0
#define SYN_CONFIG      1
#define SYN_MT_REPORT   2

// 按键代码
#define KEY_RESERVED    0
#define KEY_ESC         1
#define KEY_1           2
#define KEY_2           3
#define KEY_3           4
#define KEY_4           5
#define KEY_5           6
#define KEY_6           7
#define KEY_7           8
#define KEY_8           9
#define KEY_9           10
#define KEY_0           11
#define KEY_MINUS       12
#define KEY_EQUAL       13
#define KEY_BACKSPACE   14
#define KEY_TAB         15
#define KEY_Q           16
#define KEY_W           17
#define KEY_E           18
#define KEY_R           19
#define KEY_T           20
#define KEY_Y           21
#define KEY_U           22
#define KEY_I           23
#define KEY_O           24
#define KEY_P           25
#define KEY_LEFTBRACE   26
#define KEY_RIGHTBRACE  27
#define KEY_ENTER       28
#define KEY_LEFTCTRL    29
#define KEY_A           30
#define KEY_S           31
#define KEY_D           32
#define KEY_F           33
#define KEY_G           34
#define KEY_H           35
#define KEY_J           36
#define KEY_K           37
#define KEY_L           38
#define KEY_SEMICOLON   39
#define KEY_APOSTROPHE  40
#define KEY_GRAVE       41
#define KEY_LEFTSHIFT   42
#define KEY_BACKSLASH   43
#define KEY_Z           44
#define KEY_X           45
#define KEY_C           46
#define KEY_V           47
#define KEY_B           48
#define KEY_N           49
#define KEY_M           50
#define KEY_COMMA       51
#define KEY_DOT         52
#define KEY_SLASH       53
#define KEY_RIGHTSHIFT  54
#define KEY_KPASTERISK  55
#define KEY_LEFTALT     56
#define KEY_SPACE       57
#define KEY_CAPSLOCK    58
#define KEY_F1          59
#define KEY_F2          60
#define KEY_F3          61
#define KEY_F4          62
#define KEY_F5          63
#define KEY_F6          64
#define KEY_F7          65
#define KEY_F8          66
#define KEY_F9          67
#define KEY_F10         68
#define KEY_NUMLOCK     69
#define KEY_SCROLLLOCK  70

// 模拟内核链表
struct list_head {
    struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }
#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)

// 初始化链表
static inline void INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}

// 添加节点到链表
static inline void list_add(struct list_head *new, struct list_head *head)
{
    new->next = head->next;
    new->prev = head;
    head->next->prev = new;
    head->next = new;
}

// 从链表中删除节点
static inline void list_del(struct list_head *entry)
{
    entry->next->prev = entry->prev;
    entry->prev->next = entry->next;
    entry->next = NULL;
    entry->prev = NULL;
}

// 简化版的链表遍历宏
#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); \
         pos = n, n = pos->next)

// container_of 宏 - 修复版本
#define container_of(ptr, type, member) ({ \
        const typeof( ((type *)0)->member ) *__mptr = (ptr); \
        (type *)( (char *)__mptr - offsetof(type, member) );})

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

// list_entry 宏定义 - 修复缺失的问题
#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

// 简化版的链表遍历宏（带类型）
#define list_for_each_entry(pos, head, member) \
    for (pos = list_entry((head)->next, typeof(*pos), member); \
         &pos->member != (head); \
         pos = list_entry(pos->member.next, typeof(*pos), member))

#define list_for_each_entry_safe(pos, n, head, member) \
    for (pos = list_entry((head)->next, typeof(*pos), member), \
         n = list_entry(pos->member.next, typeof(*pos), member); \
         &pos->member != (head); \
         pos = n, n = list_entry(n->member.next, typeof(*n), member))

// ============ 前置声明 ============
struct input_dev;
struct input_handler;
struct input_handle;

// ============ Input核心结构体 ============
struct input_dev {
    char name[64];
    struct list_head node;
    struct list_head h_list;
    unsigned long evbit[1];
    unsigned long keybit[8];  // 使用unsigned long避免移位警告
};

struct input_handler {
    char name[64];
    void (*event)(struct input_handle *handle, unsigned int type, unsigned int code, int value);
    int (*connect)(struct input_handler *handler, struct input_dev *dev);
    void (*disconnect)(struct input_handle *handle);
    struct list_head node;
    struct list_head h_list;
};

struct input_handle {
    struct input_dev *dev;
    struct input_handler *handler;
    struct list_head d_node;
    struct list_head h_node;
    void *private;
};

// ============ evdev结构体 ============
struct evdev_client {
    int fd;
    struct list_head node;
    struct input_event buffer[64];
    unsigned int head;
    unsigned int tail;
};

struct evdev {
    char name[64];
    struct input_handle handle;
    struct list_head client_list;
    int minor;
    pthread_mutex_t lock;
};

// ============ joydev结构体 ============
struct joydev {
    char name[64];
    struct input_handle handle;
    int minor;
};

// ============ 函数声明 ============
// Input核心函数
int input_register_device(struct input_dev *dev);
void input_unregister_device(struct input_dev *dev);
int input_register_handler(struct input_handler *handler);
void input_unregister_handler(struct input_handler *handler);
void input_event(struct input_dev *dev, unsigned int type, unsigned int code, int value);
void input_sync(struct input_dev *dev);
void input_report_key(struct input_dev *dev, unsigned int code, int value);

// evdev函数
void evdev_init(void);
void evdev_exit(void);
void evdev_add_client(struct evdev *evdev, int fd);
void evdev_remove_client(struct evdev *evdev, int fd);
int evdev_read_client(struct evdev *evdev, int fd, struct input_event *event);

// joydev函数
void joydev_init(void);
void joydev_exit(void);

// 键盘设备函数
void keyboard_init(void);
void keyboard_start_simulation(void);
void keyboard_exit(void);

// 用户空间测试函数
void run_user_applications(void);

#endif
