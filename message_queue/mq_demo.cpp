#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <string>

class MessageQueue{
private:
    std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<std::string> queue_;
public:
    void push(const std::string& msg){
        //lock_guard简单轻量 构造时加锁 析构时解锁 不可手动解锁
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(msg);
        cv_.notify_one();                           // 通知等待的消费者线程
    }   
    
    std::string pop(){
        std::unique_lock<std::mutex> lock(mutex_);
        //cv_.wait(lock, predicate);这里队列为空时即进入等待状态 等待状态时先解锁
        //等到条件变量wait状态满足时，会内部上锁
        cv_.wait(lock, [this]{
            return !queue_.empty();
        });
        //条件成立直接进行pop操作
        //条件不成立 注册到条件变量的等待-> 自动释放互斥锁 unlock() -> 线程阻塞进入等待
        // std::string msg = queue_.front();//可以优化为右值窃取
        auto msg = std::move(queue_.front());
        queue_.pop();
        return msg;
    }

};

void producer(MessageQueue& mq){
    const std::string messages[] = {"msg1", "msg2", "msg3", "msg4"};
    for(const auto& msg : messages){
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Produced : " << msg << std::endl;
        mq.push(msg);
    }
}

void consumer(MessageQueue& mq){
    while(true){
        std::string msg = mq.pop();
        std::cout << "Consumed : " << msg << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

int main()
{
    MessageQueue mq;
    // std::ref 是一个函数模板，用于创建​​引用包装器 
    // 此外这里不能直接传递mq，直接传递的是一个副本，可能每一个线程都对自己的线程进行操作
    // 如果Message类不可拷贝的话 也可能会出错
    std::thread producer_thread(producer,std::ref(mq));
    std::thread consumer_thread(consumer,std::ref(mq));

    producer_thread.join();
    consumer_thread.join();

    
}