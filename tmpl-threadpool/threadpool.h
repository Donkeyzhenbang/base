#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <thread>
#include <functional>
#include <cassert>

class ThreadPool {
public:
    explicit ThreadPool(size_t thread_num = 8) : pool_(std::make_shared<Pool>()){
        assert(thread_num > 0); //线程数量不大于0则会触发断言
        for(size_t i = 0; i < thread_num; i ++){
            std::thread([pool = pool_]{
                std::unique_lock<std::mutex> lock(pool->mutex_);
                while(true){
                    if(!pool->tasks_.empty()){
                        auto task = std::move(pool->tasks_.front());
                        pool->tasks_.pop();
                        lock.unlock();
                        task();
                        lock.lock();
                    }
                    else if(pool->is_closed_) break;
                    else pool->cv_.wait(lock);
                }
            }).detach();
        }
    }
    ThreadPool() = default;
    ThreadPool(ThreadPool&&) = delete; //default默认移动语义

    ~ThreadPool() {
        if(static_cast<bool>(pool_)){
            {
                std::lock_guard<std::mutex> lock(pool_->mutex_);
                pool_->is_closed_ = true;
            }
            pool_->cv_.notify_all();
        }
    }

    //这里万能引用加完美转发是不是相当于没有资源的复制 本该有两次
    template<class F>   //F通常表示可调用对象
    void AddTask(F&& task){ //万能引用 既可以当左值引用 又可以当右值引用
        {
            std::lock_guard<std::mutex> lock(pool_->mutex_);
            pool_->tasks_.emplace(std::forward<F>(task));   //完美转发
        }
        pool_->cv_.notify_one(); //接收到任务响应条件变量
    }


private:
    struct Pool{
        std::mutex mutex_;
        std::condition_variable cv_;
        bool is_closed_;
        std::queue<std::function<void()>> tasks_;    //线程池工作队列
    };
    std::shared_ptr<Pool> pool_;
};



#endif // THREADPOOL_H

    //删除 复制构造 复制赋值运算符 移动构造 移动赋值
    // ThreadPool(const ThreadPool&) = delete;
    // ThreadPool& operator=(const ThreadPool&) = delete;
    // ThreadPool(ThreadPool&&) = delete;
    // ThreadPool& operator=(ThreadPool&&) = delete;

    // std::vector<std::thread> workers;
    // std::queue<std::function<void()>> tasks;
    // std::mutex queueMutex;
    // std::condition_variable condition;
    // bool stop;