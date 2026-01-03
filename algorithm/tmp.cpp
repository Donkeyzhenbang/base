#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <random>
#include <thread>
#include <type_traits>
#include <vector>
#include <chrono>

/*
  功能清单：
  - 模板化排序算法：merge_sort, heap_sort, quick_sort（支持任意可比较类型和比较器）
  - 智能指针：shared_ptr / unique_ptr 用于任务与数据管理
  - 线程安全消息队列（模板类）
  - 观察者模式：观察者注册，任务完成后通知观察者（传递 shared_ptr<vector<T>>）
  - 主程序演示：启动工作线程消费任务并通知观察者打印结果
*/

// ----------------------------
// Templates: three sorting algorithms (RandomIt iterators)
// ----------------------------

// Merge sort (stable)
template<typename RandomIt, typename Compare = std::less<typename std::iterator_traits<RandomIt>::value_type>>
void merge_sort(RandomIt first, RandomIt last, Compare comp = Compare()) {
    using T = typename std::iterator_traits<RandomIt>::value_type;
    auto n = std::distance(first, last);
    if (n <= 1) return;
    RandomIt mid = first;
    std::advance(mid, n / 2);
    merge_sort(first, mid, comp);
    merge_sort(mid, last, comp);

    std::vector<T> tmp;
    tmp.reserve(n);
    RandomIt i = first, j = mid;
    while (i != mid && j != last) {
        if (comp(*j, *i)) { // if j < i -> choose j
            tmp.push_back(*j++);
        } else {
            tmp.push_back(*i++);
        }
    }
    while (i != mid) tmp.push_back(*i++);
    while (j != last) tmp.push_back(*j++);
    std::move(tmp.begin(), tmp.end(), first);
}

// Heap sort (in-place)
template<typename RandomIt, typename Compare = std::less<typename std::iterator_traits<RandomIt>::value_type>>
void heap_sort(RandomIt first, RandomIt last, Compare comp = Compare()) {
    using diff_t = typename std::iterator_traits<RandomIt>::difference_type;
    auto n = std::distance(first, last);
    if (n <= 1) return;

    // We want a max-heap when comp = less, so use comparator adapted
    auto heapify = [&](auto idx, auto heapSize, auto&& self) -> void {
        while (true) {
            auto largest = idx;
            diff_t l = idx * 2 + 1;
            diff_t r = idx * 2 + 2;
            if (l < heapSize) {
                if (comp(*(first + largest), *(first + l))) largest = l;
            }
            if (r < heapSize) {
                if (comp(*(first + largest), *(first + r))) largest = r;
            }
            if (largest == idx) break;
            std::iter_swap(first + idx, first + largest);
            idx = largest;
        }
    };

    // build heap (as array indices 0..n-1)
    for (diff_t i = n/2 - 1; i >= 0; --i) {
        heapify(i, n, heapify);
        if (i == 0) break; // avoid underflow for unsigned
    }

    for (diff_t end = n - 1; end > 0; --end) {
        std::iter_swap(first, first + end);
        heapify(0, end, heapify);
    }
}

// Quick sort (randomized pivot)
template<typename RandomIt, typename Compare = std::less<typename std::iterator_traits<RandomIt>::value_type>>
void quick_sort(RandomIt first, RandomIt last, Compare comp = Compare()) {
    auto n = std::distance(first, last);
    if (n <= 1) return;
    // random pivot
    static thread_local std::mt19937_64 g((unsigned)std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<long long> dist(0, n - 1);
    RandomIt pivotIt = first;
    std::advance(pivotIt, (size_t)dist(g));
    auto pivot = *pivotIt;

    RandomIt i = first;
    RandomIt j = last;
    // partition: [first, i) <= pivot, (j, last) > pivot
    while (i != j) {
        while (i != j && !comp(pivot, *i)) ++i; // *i <= pivot
        while (i != j && comp(pivot, *(j - 1))) --j; // *(j-1) > pivot
        if (i != j) std::iter_swap(i, j - 1);
    }
    // now i == j, pivot position is i
    // move pivot into middle: place pivot at i by swapping with some element <= pivot
    // We'll just gather < pivot on left and >= pivot on right by stable partition approach:
    RandomIt mid = std::partition(first, last, [&](const auto& val){ return comp(val, pivot); });
    RandomIt mid2 = std::partition(mid, last, [&](const auto& val){ return !comp(pivot, val); }); // equal to pivot
    quick_sort(first, mid, comp);
    quick_sort(mid2, last, comp);
}

// ----------------------------
// Thread-safe message queue (for tasks / results)
// ----------------------------
template<typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() : closed_(false) {}

    void push(T item) {
        {
            std::lock_guard<std::mutex> lk(mu_);
            q_.push(std::move(item));
        }
        cv_.notify_one();
    }

    // pop waits until an item is available or queue is closed and empty
    bool pop(T& out) {
        std::unique_lock<std::mutex> lk(mu_);
        cv_.wait(lk, [&]{ return !q_.empty() || closed_; });
        if (q_.empty()) return false;
        out = std::move(q_.front());
        q_.pop();
        return true;
    }

    void close() {
        {
            std::lock_guard<std::mutex> lk(mu_);
            closed_ = true;
        }
        cv_.notify_all();
    }

    bool empty() {
        std::lock_guard<std::mutex> lk(mu_);
        return q_.empty();
    }

private:
    std::queue<T> q_;
    std::mutex mu_;
    std::condition_variable cv_;
    bool closed_;
};

// ----------------------------
// Observer pattern (Subject / Observer) using smart pointers
// ----------------------------
template<typename T>
class IObserver {
public:
    virtual ~IObserver() = default;
    virtual void on_notify(std::shared_ptr<T> payload) = 0;
};

template<typename T>
class Subject {
public:
    void subscribe(std::shared_ptr<IObserver<T>> obs) {
        std::lock_guard<std::mutex> lk(mu_);
        observers_.push_back(obs);
    }
    void unsubscribe_all() {
        std::lock_guard<std::mutex> lk(mu_);
        observers_.clear();
    }
    void notify(std::shared_ptr<T> payload) {
        std::lock_guard<std::mutex> lk(mu_);
        // use copy to avoid deadlocks if observers modify subject
        auto list = observers_;
        for (auto &weakObs : list) {
            if (auto obs = weakObs.lock()) {
                obs->on_notify(payload);
            }
        }
    }
private:
    std::vector<std::weak_ptr<IObserver<T>>> observers_;
    std::mutex mu_;
};

// ----------------------------
// Task definitions
// ----------------------------
enum class SortType { Merge, Heap, Quick };

template<typename T>
struct SortTask {
    SortType type;
    std::shared_ptr<std::vector<T>> data;
    std::function<void(std::shared_ptr<std::vector<T>>)> callback; // optional per-task callback
};

// ----------------------------
// Worker: consumes tasks, executes sorting, notifies subject
// ----------------------------
template<typename T>
void worker_loop(ThreadSafeQueue<std::shared_ptr<SortTask<T>>>& q, Subject<std::vector<T>>& subject, std::atomic<bool>& running) {
    while (running) {
        std::shared_ptr<SortTask<T>> task;
        if (!q.pop(task)) {
            // queue closed and empty
            break;
        }
        if (!task) continue;
        // Copy input to avoid altering original (demonstrate smart pointer usage; if we wanted in-place, we could)
        auto data_ptr = std::make_shared<std::vector<T>>(*task->data);

        // sort according to type
        switch (task->type) {
            case SortType::Merge:
                merge_sort(data_ptr->begin(), data_ptr->end());
                break;
            case SortType::Heap:
                heap_sort(data_ptr->begin(), data_ptr->end());
                break;
            case SortType::Quick:
                quick_sort(data_ptr->begin(), data_ptr->end());
                break;
        }

        // invoke per-task callback if present
        if (task->callback) {
            task->callback(data_ptr);
        }

        // notify observers about sorted result
        subject.notify(data_ptr);
    }
}

// ----------------------------
// Simple observer that prints results
// ----------------------------
template<typename T>
class PrintObserver : public IObserver<std::vector<T>> {
public:
    PrintObserver(std::string name) : name_(std::move(name)) {}
    void on_notify(std::shared_ptr<std::vector<T>> payload) override {
        std::lock_guard<std::mutex> lk(io_mu_);
        std::cout << "[" << name_ << "] Received sorted result: ";
        for (const auto& v : *payload) std::cout << v << ' ';
        std::cout << '\n';
    }
private:
    std::string name_;
    static std::mutex io_mu_;
};
template<typename T> std::mutex PrintObserver<T>::io_mu_;

// ----------------------------
// Utility: generate random vector
// ----------------------------
std::vector<int> random_vector(size_t n, int minv = 0, int maxv = 100) {
    std::vector<int> v(n);
    static std::mt19937_64 g((unsigned)std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> dist(minv, maxv);
    for (size_t i = 0; i < n; ++i) v[i] = dist(g);
    return v;
}

// ----------------------------
// Demo in main
// ----------------------------
int main() {
    using T = int;
    ThreadSafeQueue<std::shared_ptr<SortTask<T>>> queue;
    Subject<std::vector<T>> subject;
    std::atomic<bool> running(true);

    // create observers (shared_ptr), subscribe to subject
    auto obs1 = std::make_shared<PrintObserver<T>>("ObserverA");
    auto obs2 = std::make_shared<PrintObserver<T>>("ObserverB");
    subject.subscribe(obs1);
    subject.subscribe(obs2);

    // launch worker thread
    std::thread worker([&] {
        worker_loop<T>(queue, subject, running);
    });

    // produce a few tasks using unique/shared pointers to demonstrate ownership semantics
    {
        // Task 1: merge sort
        auto data1 = std::make_shared<std::vector<T>>(random_vector(10, 0, 50));
        std::cout << "Main: submitting merge sort task: ";
        for (auto v : *data1) std::cout << v << ' ';
        std::cout << '\n';

        auto task1 = std::make_shared<SortTask<T>>();
        task1->type = SortType::Merge;
        task1->data = data1;
        // per task callback example (runs in worker thread)
        static std::mutex callback_io_mu;

        task1->callback = [](std::shared_ptr<std::vector<T>> res) {
            std::lock_guard<std::mutex> lk(callback_io_mu);
            std::cout << "[Callback] Merge task done. First element: "
                    << (res->empty() ? -1 : (*res)[0]) << '\n';
        };
        queue.push(task1);
    }

    {
        // Task 2: heap sort
        auto data2 = std::make_shared<std::vector<T>>(random_vector(8, 30, 100));
        std::cout << "Main: submitting heap sort task: ";
        for (auto v : *data2) std::cout << v << ' ';
        std::cout << '\n';
        auto task2 = std::make_shared<SortTask<T>>();
        task2->type = SortType::Heap;
        task2->data = data2;
        queue.push(task2);
    }

    {
        // Task 3: quick sort
        auto data3 = std::make_shared<std::vector<T>>(random_vector(12, -20, 20));
        std::cout << "Main: submitting quick sort task: ";
        for (auto v : *data3) std::cout << v << ' ';
        std::cout << '\n';
        auto task3 = std::make_shared<SortTask<T>>();
        task3->type = SortType::Quick;
        task3->data = data3;
        queue.push(task3);
    }

    // Give worker some time to finish (in real program you'd coordinate/shutdown properly)
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // close queue and stop worker
    queue.close();
    running = false;
    if (worker.joinable()) worker.join();

    std::cout << "Main: finished.\n";
    return 0;
}
