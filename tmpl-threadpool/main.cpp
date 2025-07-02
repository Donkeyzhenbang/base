#include <atomic>
#include <future>
#include <iostream>
#include "threadpool-demo.h"
// #include "threadpool.h"

// 简化时间测量输出
template<typename T>
void PrintDuration(const char* test_name, const T& duration) {
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(duration).count() % 1000;
    std::cout << "  ⏱️ " << test_name << " duration: " << ms << "ms " << us << "us\n";
}

// 测试功能1：任务完成统计
void TestTaskCompletion() {
    ThreadPool pool(16);
    std::atomic<int> counter{0};
    const int NUM_TASKS = 3000;
    
    for (int i = 0; i < NUM_TASKS; ++i) {
        pool.AddTask([&counter] {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            counter++;
        });
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Completed tasks: " << counter << "/" << NUM_TASKS << "\n";
}

// 测试功能2：返回值处理
void TestNoModification() {
    ThreadPool pool(4);
    std::vector<std::future<int>> results;
    std::mutex result_mutex;
    std::vector<int> computed_results;
    
    for (int i = 0; i < 10; ++i) {
        pool.AddTask([i, &computed_results, &result_mutex]() {
            int result = i * i;
            {
                std::lock_guard<std::mutex> lock(result_mutex);
                computed_results.push_back(result);
            }
            std::cout << "Computed: " << i << "^2 = " << result << "\n";
        });
    }
    
    // 等待所有任务完成 (简单方式)
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 输出结果
    std::cout << "\n==== Results ====\n";
    for (int res : computed_results) {
        std::cout << "Result: " << res << "\n";
    }
}

// 测试功能3：压力测试
void StressTest() {
    const size_t THREADS = std::thread::hardware_concurrency();
    ThreadPool pool(THREADS);
    std::atomic<int> count{0};
    
    for (int i = 0; i < 100000; ++i) {
        pool.AddTask([&] {
            // 简单计算任务
            volatile int sum = 0;
            for (int j = 0; j < 10000; j++) {
                sum += j;
            }
            count++;
        });
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "Executed " << count << " tasks\n";
}

int main() {
    // 运行各种测试
    std::cout << "=== Task Completion Test ===\n";
    auto test_start = std::chrono::high_resolution_clock::now();
    TestTaskCompletion();
    auto task_time = std::chrono::high_resolution_clock::now();
    PrintDuration("TestTaskCompletion test", task_time - test_start);
    
    // std::cout << "\n=== Return Values Test ===\n";
    // TestNoModification();
    
    std::cout << "\n=== Stress Test ===\n";
    auto test_start2 = std::chrono::high_resolution_clock::now();
    StressTest();
    auto task_time2 = std::chrono::high_resolution_clock::now();
    PrintDuration("StressTest test", task_time2 - test_start2);
    
    return 0;
}

/*
auto test_start = std::chrono::high_resolution_clock::now();
    TestTaskCompletion();
    auto task_time = std::chrono::high_resolution_clock::now();
    PrintDuration("TaskCompletion test", task_time - test_start);
*/