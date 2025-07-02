#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>
#include <functional>
#include <cassert>

class ThreadPool{
public:
    explicit ThreadPool(size_t thread_num = 8) : pool_(std::make_shared<Pool>()) {
        assert(thread_num > 0);
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
                    else pool->cv_.wait(lock, [pool]{
                        return !pool->tasks_.empty() || pool->is_closed_;
                    });
                }
            }).detach();
        }
    }

    ThreadPool() = delete;
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    ~ThreadPool(){
        if(static_cast<bool>(pool_)){
            {
                std::lock_guard<std::mutex> lock(pool_->mutex_);
                pool_->is_closed_ = true;
            }
            pool_->cv_.notify_all();
        }
    }

    
    template <typename F>
    void AddTask(F&& task){  //完美转发+万能引用
        {
            std::lock_guard<std::mutex> lock(pool_->mutex_);
            pool_->tasks_.emplace(std::forward<F>(task)); //完美转发
        }
        pool_->cv_.notify_one();
    }

private:
    struct Pool
    {
        std::mutex mutex_;
        std::condition_variable cv_;
        std::queue<std::function<void()>> tasks_;
        bool is_closed_;
    };
    std::shared_ptr<Pool> pool_; 
};



#endif //  THREADPOOL_H
