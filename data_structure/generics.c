#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 定义错误码，增强错误处理能力
typedef enum {
    CONTAINER_OK = 0,
    CONTAINER_ERR_OUT_OF_RANGE,
    CONTAINER_ERR_MEMORY_ALLOC,
    CONTAINER_ERR_EMPTY
} ContainerError;

// 泛型容器结构 - 使用void*存储任意类型数据
typedef struct GenericContainer {
    void* data;               // 存储数据的通用指针
    size_t elementSize;       // 每个元素的大小（字节）
    size_t size;              // 当前元素数量
    size_t capacity;          // 容量
    // 函数指针：用于打印元素（不同类型有不同实现）
    void (*printElement)(const void*);
} GenericContainer;

// 初始化容器
ContainerError container_init(GenericContainer* container, size_t elementSize, 
                             size_t initCapacity, void (*printFunc)(const void*)) {
    container->elementSize = elementSize;
    container->size = 0;
    container->capacity = initCapacity;
    container->printElement = printFunc;  // 绑定打印函数
    
    // 分配内存
    container->data = malloc(elementSize * initCapacity);
    if (container->data == NULL) {
        return CONTAINER_ERR_MEMORY_ALLOC;
    }
    return CONTAINER_OK;
}

// 销毁容器
void container_destroy(GenericContainer* container) {
    free(container->data);
    container->data = NULL;
    container->size = 0;
    container->capacity = 0;
    container->printElement = NULL;
}

// 扩展容量
ContainerError container_reserve(GenericContainer* container, size_t newCapacity) {
    if (newCapacity <= container->capacity) {
        return CONTAINER_OK;
    }
    
    // 分配新内存 - 使用void*处理任意类型
    void* newData = realloc(container->data, container->elementSize * newCapacity);
    if (newData == NULL) {
        return CONTAINER_ERR_MEMORY_ALLOC;
    }
    
    container->data = newData;
    container->capacity = newCapacity;
    return CONTAINER_OK;
}

// 添加元素到末尾 - 通过void*接收任意类型元素
ContainerError container_push_back(GenericContainer* container, const void* element) {
    // 检查容量，不足则扩容
    if (container->size >= container->capacity) {
        size_t newCapacity = (container->capacity == 0) ? 4 : container->capacity * 2;
        ContainerError err = container_reserve(container, newCapacity);
        if (err != CONTAINER_OK) {
            return err;
        }
    }
    
    // 计算目标位置并复制数据
    char* target = (char*)container->data + container->elementSize * container->size;
    memcpy(target, element, container->elementSize);
    container->size++;
    
    return CONTAINER_OK;
}

// 获取指定位置的元素 - 返回void*，由调用者负责类型转换
ContainerError container_get(const GenericContainer* container, size_t index, void**element) {
    if (index >= container->size) {
        *element = NULL;
        return CONTAINER_ERR_OUT_OF_RANGE;
    }
    
    *element = (char*)container->data + container->elementSize * index;
    return CONTAINER_OK;
}

// 泛型查找函数 - 使用函数指针实现类型特定的比较逻辑
void* container_find(const GenericContainer* container, const void* key,
                    int (*compare)(const void*, const void*)) {
    for (size_t i = 0; i < container->size; i++) {
        void* element = (char*)container->data + container->elementSize * i;
        if (compare(element, key) == 0) {
            return element;
        }
    }
    return NULL;
}

// 打印容器所有元素 - 利用存储的打印函数指针
void container_print(const GenericContainer* container) {
    if (container->size == 0) {
        printf("Container is empty\n");
        return;
    }
    
    printf("Container elements: ");
    for (size_t i = 0; i < container->size; i++) {
        const void* element = (char*)container->data + container->elementSize * i;
        container->printElement(element);  // 调用类型特定的打印函数
        printf(" ");
    }
    printf("\n");
}

// --------------------------
// 以下是针对具体类型的实现，展示如何适配泛型容器
// --------------------------

// 1. 整数类型相关实现
// 打印整数的函数（符合printElement函数指针类型）
void printInt(const void* element) {
    printf("%d", *(const int*)element);  // 显式类型转换
}

// 比较整数的函数（符合compare函数指针类型）
int compareInt(const void* a, const void* b) {
    int valA = *(const int*)a;
    int valB = *(const int*)b;
    return (valA > valB) - (valA < valB);
}

// 整数容器测试
void testIntContainer() {
    printf("=== Testing Integer Container ===\n");
    
    GenericContainer intContainer;
    // 初始化：指定元素大小、初始容量和打印函数
    ContainerError err = container_init(&intContainer, sizeof(int), 5, printInt);
    if (err != CONTAINER_OK) {
        printf("Failed to initialize int container\n");
        return;
    }
    
    // 添加元素（通过void*传递任意类型）
    int values[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        container_push_back(&intContainer, &values[i]);  // 传递int的地址
    }
    
    // 打印所有元素（使用绑定的打印函数）
    container_print(&intContainer);
    
    // 查找元素（使用整数比较函数）
    int key = 30;
    int* found = (int*)container_find(&intContainer, &key, compareInt);
    if (found) {
        printf("Found integer: %d\n", *found);
    } else {
        printf("Integer %d not found\n", key);
    }
    
    // 获取指定位置元素
    void* element;
    if (container_get(&intContainer, 2, &element) == CONTAINER_OK) {
        printf("Element at index 2: %d\n", *(int*)element);  // 显式类型转换
    }
    
    container_destroy(&intContainer);
    printf("\n");
}

// 2. 自定义结构体类型相关实现
typedef struct {
    char name[20];
    int age;
} Person;

// 打印Person的函数（符合printElement函数指针类型）
void printPerson(const void* element) {
    const Person* p = (const Person*)element;  // 显式类型转换
    printf("%s(%d)", p->name, p->age);
}

// 比较Person的函数（按年龄比较，符合compare函数指针类型）
int comparePersonByAge(const void* a, const void* b) {
    const Person* p1 = (const Person*)a;
    const Person* p2 = (const Person*)b;
    return p1->age - p2->age;
}

// 结构体容器测试
void testPersonContainer() {
    printf("=== Testing Person Container ===\n");
    
    GenericContainer personContainer;
    // 初始化：指定结构体大小、初始容量和打印函数
    ContainerError err = container_init(&personContainer, sizeof(Person), 3, printPerson);
    if (err != CONTAINER_OK) {
        printf("Failed to initialize person container\n");
        return;
    }
    
    // 添加结构体元素
    Person p1 = {"Alice", 30};
    Person p2 = {"Bob", 25};
    Person p3 = {"Charlie", 35};
    container_push_back(&personContainer, &p1);  // 传递结构体地址
    container_push_back(&personContainer, &p2);
    container_push_back(&personContainer, &p3);
    
    // 打印所有元素
    container_print(&personContainer);
    
    // 查找元素（使用Person比较函数）
    Person searchPerson = {"", 25};  // 只关心年龄字段
    Person* foundPerson = (Person*)container_find(&personContainer, &searchPerson, comparePersonByAge);
    if (foundPerson) {
        printf("Found person: ");
        printPerson(foundPerson);
        printf("\n");
    }
    
    // 修改元素（通过指针直接操作）
    void* element;
    if (container_get(&personContainer, 0, &element) == CONTAINER_OK) {
        Person* p = (Person*)element;  // 显式类型转换
        strcpy(p->name, "Alicia");     // 修改找到的元素
        printf("After modification: ");
        printPerson(element);
        printf("\n");
    }
    
    container_destroy(&personContainer);
    printf("\n");
}

int main() {
    // 测试两种不同类型的容器
    testIntContainer();
    testPersonContainer();
    
    return 0;
}
