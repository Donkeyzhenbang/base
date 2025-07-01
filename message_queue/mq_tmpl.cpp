#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <optional>
#include <type_traits>
#include <utility>

template <typename T>
class MessageQueue{
public:
    // 共有接口方法
    void push(const T& msg){
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(msg);
        cv_.notify_one();
    }
    //移动方法
    void push(T&& msg){
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(msg));
        cv_.notify_one();
    }
    T pop(){
        std::unique_lock<std::mutex> lock(mutex_);
        // 收到新消息或者停止信号都会被唤醒
        // 1. 在等待时自动释放锁（避免死锁）
        // 2. 被唤醒时重新获取锁
        cv_.wait(lock, [this]{
            return !queue_.empty() || should_stop_;
        });
        if(should_stop_ && queue_.empty()){
            throw std::runtime_error("Queue Stoped");
        }
        T out = std::move(queue_.front());
        queue_.pop();
        return out;
    }

    // 非阻塞弹出
    bool try_pop(T& result){
        std::lock_guard<std::mutex> lock(mutex_);
        if(queue_.empty()) return false;
        result = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    // 停止队列
    void stop(){
        std::lock_guard<std::mutex> lock(mutex_);
        should_stop_ = true;
        cv_.notify_all(); //唤醒所有消费者
    }
private:
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<T> queue_;
    bool should_stop_ = false;
};


/***************消费者 生产者模板*******************/
template <typename T, typename MessageGenerator>
void producer(MessageQueue<T>& mq, MessageGenerator&& generator, int count){
    for(int i = 0; i < count; i ++){
        T msg = generator(i);
        std::cout << "Producing: " << msg << std::endl;
        mq.push(std::move(msg));        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

template <typename T, typename MessageProcessor>
void consumer(MessageQueue<T>& mq, MessageProcessor&& processor){
    try{
        while(true){
            T msg = mq.pop();
            std::cout << "Consumed : ";
            processer(msg);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } catch (const std::runtime_error& e){
        std::cout << "Consumer stopping ： " << e.what() << std::endl;
    }
}

/***************消息生成器*******************/
std::string string_generator(int index){
    return "Message" + std::to_string(index + 1);
}

struct CustomMessage {
    int id;
    double value;
    //重载输出流
    friend std::ostream& operator<<(std::ostream& os, const CustomMessage& msg){
        return os << "Custom[" << msg.id << ": " << msg.value << "]";
    } 
};

CustomMessage custom_generator(int index){
    return {index + 1, 10.0 * (index + 1)};
}

/***************包装函数解决线程创建问题*******************/
//! std::thread 构造函数无法正确推导模板函数 producer 和 consumer 的模板参数类型
template <typename T, typename Generator>
void producer_wrapper(MessageQueue<T>* mq, Generator generator, int count) {
    producer<T>(*mq, generator, count);
}



template <typename T, typename Processor>
void consumer_wrapper(MessageQueue<T>* mq, Processor processer){
    consumer<T>(*mq, processer);
}

int main()
{
    /*
    MessageQueue<std::string> mq;
    // std::ref 是一个函数模板，用于创建​​引用包装器 
    //! 此外这里不能直接传递mq，直接传递的是一个副本，可能每一个线程都对自己的线程进行操作
    // 如果Message类不可拷贝的话 也可能会出错
    std::thread producer_thread(producer,std::ref(mq));
    std::thread consumer_thread(consumer,std::ref(mq));

    producer_thread.join();
    consumer_thread.join();
    */
    // 字符串
    {
        MessageQueue<std::string> string_queue;
        bool stop_flag = false;
        std::thread string_producer(
            producer_wrapper<std::string, decltype(string_generator)>,
            &string_queue,
            string_generator,
            5
        );

        std::thread string_consumer([&]{
            while(true){
                try{
                    auto msg = string_queue.pop();
                    std::cout << "Consumed : " << msg << " (string)" << std::endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(800));
                }catch (const std::runtime_error& e){
                    std::cout << "Consuemer stopped : " << e.what() << std::endl;
                    break;
                }
            }
        });

        string_producer.join();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        string_queue.stop();
        string_consumer.join();

        std::cout << "\n=============\n";

    }

    // 自定义结构体
    {
        MessageQueue<CustomMessage> custom_queue;
        std::thread custom_producer(
            producer_wrapper<CustomMessage, decltype(custom_generator)>,
            &custom_queue,
            custom_generator,
            4
        );
        // std::thread custom_producer([&]{
        //     for(int i = 0; i < 4; i ++){
        //         auto msg = custom_generator(i);
        //         std::cout << "Producing:" << msg << std::endl;
        //         custom_queue.push(msg);
        //         std::this_thread::sleep_for(std::chrono::milliseconds(500));
        //     }
        // });

        std::thread custom_consumer([&]{
            try{
                while(true){
                    auto msg = custom_queue.pop();
                    std::cout << "Consumed : " << msg << " (custom)" << std::endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(800));
                }
            } catch (const std::runtime_error& e){
                std::cout << "Consumer stopped" << e.what() << std::endl;
            }
        });

        custom_producer.join();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        custom_queue.stop();
        custom_consumer.join();

    }
    
    
    return 0;


    
}