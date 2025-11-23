#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>
#include <functional>

class ThreadPool {
public:
    explicit ThreadPool(size_t num = 8) {
        for(size_t i = 0; i < num; i ++ ){
            std::thread(){
                
            }
        }
    }

    ~ThreadPool() {
        if(static_cast<bool>(pool_)){
            std::lock_guard<std::mutex> lock(pool_->mtx);
            pool_->is_closed = true;
        }
        pool_->cv.notify_one();
    }

    template <typename F>
    void AddTask(F&& task) {
        {
            std::lock_guard<std::mutex> lock(pool_->mtx);
            pool_->task_.emplace(std::forward<F>(task));
        }
        pool_->cv.notify_one();
    }


private:
    struct Pool{
        std::mutex mtx;
        std::condition_variable cv;
        std::queue<std::function<void()>> task_;
        bool is_closed;
    };
    std::shared_ptr<Pool> pool_;
};

#endif //  THREADPOOL_H