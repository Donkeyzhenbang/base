// 文件: mpmc_examples_fixed.cpp
// 编译: g++ -std=c++17 -O2 -pthread mpmc_examples_fixed.cpp -o mpmc_examples_fixed
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>
#include <condition_variable>
#include <chrono>
#include <string>

// ---------------------- 1) Vyukov MPMC 有界无锁队列 (保持原样) ----------------------
template <typename T>
class MPMCQueue {
public:
    explicit MPMCQueue(size_t capacity) {
        size_t cap = 1;
        while (cap < capacity) cap <<= 1;
        capacity_ = cap;
        mask_ = capacity_ - 1;
        buffer_ = static_cast<Cell*>(operator new[](sizeof(Cell) * capacity_));
        for (size_t i = 0; i < capacity_; ++i) new (&buffer_[i]) Cell(i);
        enqueue_pos_.store(0, std::memory_order_relaxed);
        dequeue_pos_.store(0, std::memory_order_relaxed);
    }
    ~MPMCQueue() {
        for (size_t i = 0; i < capacity_; ++i) buffer_[i].~Cell();
        operator delete[](buffer_);
    }

    bool try_enqueue(const T& value) { return try_enqueue_impl(value); }
    bool try_enqueue(T&& value) { return try_enqueue_impl(std::move(value)); }

    bool try_dequeue(T& out) {
        size_t pos = dequeue_pos_.load(std::memory_order_relaxed);
        for (;;) {
            Cell* cell = &buffer_[pos & mask_];
            size_t seq = cell->seq.load(std::memory_order_acquire);
            intptr_t dif = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos + 1);
            if (dif == 0) {
                if (dequeue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
                    out = std::move(cell->data);
                    cell->seq.store(pos + capacity_, std::memory_order_release);
                    return true;
                }
            } else if (dif < 0) {
                return false;
            } else {
                pos = dequeue_pos_.load(std::memory_order_relaxed);
            }
        }
    }

private:
    struct Cell {
        std::atomic<size_t> seq;
        T data;
        Cell(size_t s) : seq(s), data() {}
    };

    template <typename U>
    bool try_enqueue_impl(U&& value) {
        size_t pos = enqueue_pos_.load(std::memory_order_relaxed);
        for (;;) {
            Cell* cell = &buffer_[pos & mask_];
            size_t seq = cell->seq.load(std::memory_order_acquire);
            intptr_t dif = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos);
            if (dif == 0) {
                if (enqueue_pos_.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
                    cell->data = std::forward<U>(value);
                    cell->seq.store(pos + 1, std::memory_order_release);
                    return true;
                }
            } else if (dif < 0) {
                return false;
            } else {
                pos = enqueue_pos_.load(std::memory_order_relaxed);
            }
        }
    }

    size_t capacity_;
    size_t mask_;
    Cell* buffer_;
    alignas(64) std::atomic<size_t> enqueue_pos_;
    alignas(64) std::atomic<size_t> dequeue_pos_;
};

// ---------------------- 2) 阻塞队列（支持 close()，避免消费者永久阻塞） ----------------------
template <typename T>
class BlockingQueue {
public:
    explicit BlockingQueue(size_t capacity)
        : capacity_(capacity), head_(0), tail_(0), size_(0), closed_(false) {
        buffer_.resize(capacity_);
    }

    // 阻塞入队；如果队列已 close() 则返回 false 表示拒绝入队
    bool enqueue(T item) {
        std::unique_lock<std::mutex> lk(mutex_);
        not_full_.wait(lk, [this]{ return size_ < capacity_ || closed_; });
        if (closed_) return false;
        buffer_[tail_] = std::move(item);
        tail_ = (tail_ + 1) % capacity_;
        ++size_;
        lk.unlock();
        not_empty_.notify_one();
        return true;
    }

    // 阻塞出队：如果成功将结果放入 out 返回 true；若队列已关闭且为空返回 false 表示没有更多数据
    bool dequeue(T& out) {
        std::unique_lock<std::mutex> lk(mutex_);
        not_empty_.wait(lk, [this]{ return size_ > 0 || closed_; });
        if (size_ == 0 && closed_) {
            return false; // 已关闭且没有元素，通知调用者退出
        }
        out = std::move(buffer_[head_]);
        head_ = (head_ + 1) % capacity_;
        --size_;
        lk.unlock();
        not_full_.notify_one();
        return true;
    }

    // 封闭队列：之后不会再enqueue，唤醒所有等待的消费者/生产者
    void close() {
        std::lock_guard<std::mutex> lk(mutex_);
        closed_ = true;
        not_empty_.notify_all();
        not_full_.notify_all();
    }

    bool is_closed() const {
        std::lock_guard<std::mutex> lk(mutex_);
        return closed_;
    }

private:
    size_t capacity_;
    std::vector<T> buffer_;
    size_t head_, tail_, size_;
    mutable std::mutex mutex_;
    std::condition_variable not_empty_, not_full_;
    bool closed_;
};

// ---------------------- 示例应用场景：日志汇总 (多生产者 -> 多消费者) ----------------------
struct LogItem {
    int producer_id;
    int seq;
    std::string text;
};

void example_using_blocking_queue() {
    std::cout << "=== BlockingQueue 示例 (修复后) ===\n";
    BlockingQueue<LogItem> q(128);
    const int PRODUCERS = 4;
    const int CONSUMERS = 3;
    const int ITEMS_PER_PRODUCER = 1000;

    std::atomic<int> produced{0};
    std::atomic<int> consumed{0};

    // 生产者
    std::vector<std::thread> producers;
    for (int p = 0; p < PRODUCERS; ++p) {
        producers.emplace_back([p, &q, &produced, ITEMS_PER_PRODUCER](){
            for (int i = 0; i < ITEMS_PER_PRODUCER; ++i) {
                LogItem it{p, i, "log message from producer " + std::to_string(p)};
                // enqueue 失败只会在队列被 close() 后发生（本示例不会在生产中途 close）
                while (!q.enqueue(std::move(it))) {
                    // 如果拒绝入队（队列被关闭），直接退出
                    return;
                }
                ++produced;
            }
        });
    }

    // 消费者：使用 dequeue 返回值作为退出条件（避免在阻塞前检查外部计数）
    std::vector<std::thread> consumers;
    for (int c = 0; c < CONSUMERS; ++c) {
        consumers.emplace_back([c, &q, &consumed](){
            LogItem it;
            while (q.dequeue(it)) {
                // process (此处只计数)
                ++consumed;
                // 可模拟 I/O 延迟
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
            // dequeue 返回 false 表示队列已关闭且空，消费者可正常退出
        });
    }

    for (auto &t : producers) t.join();
    // 所有生产者结束后，主线程 close 队列，唤醒所有仍在等待的消费者
    q.close();

    for (auto &t : consumers) t.join();

    std::cout << "BlockingQueue finished: produced=" << produced.load() << " consumed=" << consumed.load() << "\n";
}

void example_using_mpmc_queue() {
    std::cout << "=== MPMCQueue (无锁) 示例 ===\n";
    MPMCQueue<LogItem> q(256);
    const int PRODUCERS = 4;
    const int CONSUMERS = 3;
    const int ITEMS_PER_PRODUCER = 100000;

    std::atomic<int> produced{0};
    std::atomic<int> consumed{0};

    // 生产者
    std::vector<std::thread> producers;
    for (int p = 0; p < PRODUCERS; ++p) {
        producers.emplace_back([p, &q, &produced, ITEMS_PER_PRODUCER](){
            for (int i = 0; i < ITEMS_PER_PRODUCER; ++i) {
                LogItem it{p, i, "log msg"};
                while (!q.try_enqueue(it)) {
                    std::this_thread::yield();
                }
                ++produced;
            }
        });
    }

    // 消费者：自旋式（非阻塞 try_dequeue + yield）
    std::vector<std::thread> consumers;
    for (int c = 0; c < CONSUMERS; ++c) {
        consumers.emplace_back([c, &q, &consumed, &produced, PRODUCERS, ITEMS_PER_PRODUCER](){
            int expect_total = PRODUCERS * ITEMS_PER_PRODUCER;
            while (consumed < expect_total) {
                LogItem it;
                if (q.try_dequeue(it)) {
                    ++consumed;
                } else {
                    std::this_thread::yield();
                }
            }
        });
    }

    for (auto &t : producers) t.join();
    for (auto &t : consumers) t.join();

    std::cout << "MPMCQueue finished: produced=" << produced.load() << " consumed=" << consumed.load() << "\n";
}

int main() {
    example_using_blocking_queue();
    example_using_mpmc_queue();
    return 0;
}
