## 消息队列

### 设计思路

![alt text](assets/message.png)

### std::ref

![alt text](assets/std-ref.png)

### const T&

- 避免拷贝
- 保证对象不被修改
- 接收临时对象 const引用可以接收临时对象 比如右值
- 避免对象切割 按值传递时派生类对象会发生对象切割 引用可以保持多态行为

```cpp
class Animal { public: virtual void sound() const = 0; };
class Dog : public Animal { public: void sound() const override { std::cout << "Woof!"; } };

void slice(Animal a) { a.sound(); }  // 错误：只调用Animal的sound
void proper(const Animal& a) { a.sound(); } // 正确：多态调用

int main() {
    Dog dog;
    slice(dog);     // 输出（错误）：无声音（纯虚函数调用错误）
    proper(dog);    // 输出（正确）："Woof!"
}
```
### 右值窃取
```cpp
    std::string msg = queue_.front();   //可以优化为右值窃取
    auto msg = std::move(queue_.front());
```

### lock_guard && unique_lock
*lock_guard*
- 简单轻量（RAII 封装）
- 无额外状态，仅栈空间（约 8-16 字节）
- 构造时加锁，析构时解锁
​- ​不可手动解锁​​
- 性能：lock_guard 比手写 lock/unlock 快约 2-3ns

*unique_lock*
- 状态机模型（管理锁状态）
- 额外状态：锁状态、是否拥有（约 32-48 字节）
- 支持手动锁/解锁（lock()/unlock()）
- 支持延迟锁定（defer_lock 选项）
- 支持条件变量等待
- 可转移所有权（移动语义）
- 性能：比 lock_guard 慢约 5-10ns（用于条件变量时必要开销）



### 条件变量condition_variable

条件变量中必须使用unique_lock 其内部需要手动加锁解锁，lock_guard不支持 `cv_.wait(lock, predicate)`等价于
```cpp
while (!predicate()) {
    // 原子操作：解锁 + 等待
    cv_.wait_internal(lock);
    // 唤醒后重新加锁
}
```

内部流程示意
```cpp
void condition_variable::wait(unique_lock<mutex>& lk, Predicate pred) {
    while (!pred()) {
        // 1. 解锁关联的mutex
        lk.unlock(); 
        // 2. 进入等待状态（内核对象）
        wait_internal();
        // 3. 唤醒后重新加锁
        lk.lock();
    }
}
```

### decltype
- decltype用于编译时类型推导
- 常用于泛型编程结合auto，用于追踪函数的返回值类型

### 线程函数无法推导模板类型

- std::thread构造函数尝试推导生产者函数类型，如果不是明确函数实例则会编译错误，是则成功创建线程
- 这里使用lambda表达式是最好的

*失败代码*
```cpp
MessageQueue<CustomMessage> custom_queue;

std::thread custom_producer(
    producer<CustomMessage>,      // ❌ 问题所在
    std::ref(custom_queue),
    custom_generator,
    4
);
```

*成功代码*
```cpp
template <typename T, typename Generator>
void producer_wrapper(MessageQueue<T>* mq, Generator generator, int count) {
    producer<T>(*mq, generator, count);
}

// 使用
std::thread custom_producer(
    producer_wrapper<CustomMessage, decltype(custom_generator)>, // ✅
    &custom_queue,
    custom_generator,
    4
);
```


1. producer<CustomMessage> 是什么
- 它是一个​​函数模板​​，不是具体的函数实例
- 模板签名：template <typename Generator> void producer(...)

2. ​​std::thread 的限制：
- std::thread 构造函数要求​​具体函数指针或可调用对象​​
- 不能直接传递​​部分特化的模板​

3. 之前也是这个producer模板类，之前的模板初始化确实是不完整的，没有明确实例化


### 模板特化
- 模板特化是 C++ 模板编程中的高级技术，它允许开发者​​为特定类型提供自定义实现​​。
- 全特化    为所有模板参数指定具体类型
- 部分特化  为部分模板参数指定具体类型

特化的工作机制
- 检查是否有完全匹配的特化
- 检查是否有部分特化
- 使用通用模板

```cpp
// 通用模板
template<typename T>
void process(T obj) { 
    // 通用逻辑 
}

// int 类型的全特化
template<>
void process<int>(int num) {
    // 为int优化的逻辑
}

// 指针类型的部分特化
template<typename U>
void process(U* ptr) {
    // 为指针优化的逻辑
}
```

#### 完全特化
具体到某个类型
```cpp
#include <iostream>
#include <string>

// 通用模板定义
template <typename T>
void process(T value) {
    std::cout << "通用处理: " << value << std::endl;
}

// 完整特化：int类型
template <>
void process<int>(int value) {
    std::cout << "int 特化: " << value * 2 << std::endl;
}

// 完整特化：std::string类型
template <>
void process<std::string>(std::string value) {
    std::cout << "字符串 特化: Hello " << value << "!" << std::endl;
}

int main() {
    process(3.14);      // 通用处理: 3.14
    process(10);        // int 特化: 20
    process("World");   // 字符串 特化: Hello World!
}
```

#### 部分特化
对于部分类型进行特化，也是没有完全指定具体类型

```cpp
#include <iostream>
#include <vector>
#include <type_traits>

// 通用模板定义
template <typename T>
struct Processor {
    static void execute(T value) {
        std::cout << "通用处理器: " << value << std::endl;
    }
};

// 部分特化：指针类型
template <typename T>
struct Processor<T*> {
    static void execute(T* value) {
        std::cout << "指针特化: 地址=" << value 
                  << " 值=" << *value << std::endl;
    }
};

// 部分特化：容器类型
template <typename T>
struct Processor<std::vector<T>> {
    static void execute(const std::vector<T>& vec) {
        std::cout << "容器特化: ";
        for (const auto& item : vec) {
            std::cout << item << " ";
        }
        std::cout << std::endl;
    }
};

// 实际使用代码
int main() {
    int num = 42;
    std::vector<std::string> words {"C++", "Templates", "Partial", "Specialization"};
    
    Processor<int>::execute(10);         // 通用处理器: 10
    Processor<int*>::execute(&num);     // 指针特化: 地址=0x7ffdf9b82a54 值=42
    Processor<decltype(words)>::execute(words); // 容器特化: C++ Templates Partial Specialization 
}
```

### optional
std::optional 是 C++17 引入的一个模板类，用于表示一个可能包含值的对象。它提供了一种类型安全的方式来表示"可能有值"或"没有值"的情况，避免了使用特殊值（如 -1、nullptr 等）或额外的布尔变量。
- std::optional 提供了一种类型安全、表达力强的方式来处理可能缺失的值，是现代 C++ 中处理可选值的推荐方式。

std::optional<T> 可以：
- 包含一个类型为 T 的值
- 或者不包含任何值（空状态）

它特别适合用于以下场景：
- 可能失败的函数返回值
- 可选的数据成员
- 延迟初始化
- 查找操作（可能找不到结果）

```cpp
#include <iostream>
#include <optional>
#include <string>
#include <vector>
#include <functional>

// 查找函数：可能找不到元素
std::optional<size_t> findIndex(const std::vector<int>& vec, int value) {
    for (size_t i = 0; i < vec.size(); ++i) {
        if (vec[i] == value) {
            return i;
        }
    }
    return std::nullopt;
}

// 复杂对象构造
class Configuration {
public:
    Configuration(int level, const std::string& name) 
        : level_(level), name_(name) {}
    
    void print() const {
        std::cout << "Config: " << name_ << " (Level " << level_ << ")\n";
    }

private:
    int level_;
    std::string name_;
};

// 工厂函数：可能返回空
std::optional<Configuration> loadConfiguration(bool valid) {
    if (valid) {
        return Configuration{5, "Production"};
    }
    return std::nullopt;
}

int main() {
    // 1. 基本使用
    std::optional<int> maybeNumber;
    std::cout << "maybeNumber has value: " << maybeNumber.has_value() << "\n";
    
    maybeNumber = 42;
    if (maybeNumber) {
        std::cout << "Number is: " << *maybeNumber << "\n";
    }
    
    // 2. 使用 value_or
    std::optional<std::string> maybeName;
    std::cout << "Name: " << maybeName.value_or("Unknown") << "\n";
    maybeName = "Alice";
    std::cout << "Name: " << maybeName.value_or("Unknown") << "\n";
    
    // 3. 函数返回值处理
    std::vector<int> numbers = {10, 20, 30, 40, 50};
    auto index = findIndex(numbers, 30);
    
    if (index) {
        std::cout << "Found at index: " << *index << "\n";
    } else {
        std::cout << "Value not found\n";
    }
    
    // 4. 使用 emplace 原地构造
    std::optional<Configuration> config;
    config.emplace(3, "Development"); // 直接构造对象
    config->print();
    
    // 5. 高级：链式操作（C++23）
    auto validConfig = loadConfiguration(true);
    auto invalidConfig = loadConfiguration(false);
    
    std::cout << "\nValid config: ";
    if (validConfig) validConfig->print();
    
    std::cout << "Invalid config: ";
    if (!invalidConfig) std::cout << "Not available\n";
    
    // 6. 异常处理
    try {
        auto badAccess = invalidConfig.value(); // 抛出异常
    } catch (const std::bad_optional_access& e) {
        std::cout << "\nCaught exception: " << e.what() << "\n";
    }
    
    // 7. 交换操作
    std::optional<int> a = 10;
    std::optional<int> b = 20;
    std::cout << "\nBefore swap: a=" << *a << ", b=" << *b << "\n";
    a.swap(b);
    std::cout << "After swap: a=" << *a << ", b=" << *b << "\n";
    
    // 8. 重置值
    a.reset();
    std::cout << "After reset: a has value? " << a.has_value() << "\n";
    
    return 0;
}

常用api

检查值是否存在
std::optional<std::string> name = "Alice";

// 使用 has_value()
if (name.has_value()) {
    // 有值时的处理
}

// 使用 bool 转换
if (name) {
    // 有值时的处理
}

// 检查是否为空
if (!name) {
    // 空状态处理
}

访问值
std::optional<double> price = 19.99;

// 使用 value() - 如果为空会抛出 std::bad_optional_access
double p = price.value();

// 使用 value_or() - 提供默认值
double safePrice = price.value_or(0.0);

// 使用 * 运算符（不解引用空值）
if (price) {
    double p = *price;
}

// 使用 -> 运算符访问成员
std::optional<std::string> text = "Hello";
if (text) {
    size_t len = text->length();
}


```