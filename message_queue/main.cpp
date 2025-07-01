#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

template<typename T>
class MessageQueue {
public:
    // 优化1: 支持超时等待
    std::optional<T> pop_for(std::chrono::milliseconds timeout) {
        std::unique_lock lock(mutex_);
        if(!cv_.wait_for(lock, timeout, [this]{ return !queue_.empty(); })) {
            return std::nullopt; // 超时返回空值
        }
        return pop_internal();
    }

    // 优化2: 非阻塞版pop
    std::optional<T> try_pop() {
        std::lock_guard lock(mutex_);
        if(queue_.empty()) return std::nullopt;
        return pop_internal();
    }

    // 优化3: 万能引用+完美转发
    template<typename U>
    void push(U&& msg) {
        {
            std::lock_guard lock(mutex_);
            queue_.push(std::forward<U>(msg));
        }
        cv_.notify_one();
    }

    // 优化4: 批量推送
    template<typename InputIt>
    void push_bulk(InputIt first, InputIt last) {
        if(first == last) return;
        
        {
            std::lock_guard lock(mutex_);
            for(; first != last; ++first) {
                queue_.push(std::move(*first));
            }
        }
        cv_.notify_all(); // 唤醒所有消费者
    }

private:
    // 内部统一弹出逻辑
    T pop_internal() {
        T msg = std::move(queue_.front());
        queue_.pop();
        return msg;
    }

    std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<T> queue_;
};