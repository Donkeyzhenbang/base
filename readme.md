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

条件变量中必须使用unique_lock `cv_.wait(lock, predicate)`等价于
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