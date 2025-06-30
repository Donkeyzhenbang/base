#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <cstdlib>

using namespace boost::interprocess;

struct SharedData {
    int counter;
    char message[256];
};

int main() {
    const char* SHARED_MEMORY_NAME = "MySharedMemory";
    const char* MUTEX_NAME = "MySharedMutex";
    const size_t SHARED_MEMORY_SIZE = 1024;

    // 清理之前的残留资源（重要！）
    try {
        named_mutex::remove(MUTEX_NAME);
    } catch(...) {}
    try {
        shared_memory_object::remove(SHARED_MEMORY_NAME);
    } catch(...) {}

    try {
        // 创建共享内存
        managed_shared_memory segment(create_only, SHARED_MEMORY_NAME, SHARED_MEMORY_SIZE);
        
        // 创建互斥锁
        named_mutex mutex(create_only, MUTEX_NAME);

        // 初始化共享数据
        SharedData* data = segment.construct<SharedData>("SharedData")();
        data->counter = 0;
        strcpy(data->message, "Initial Message");

        // 原子标志确保输出顺序
        std::atomic<bool> writer_done(false);
        
        std::thread writer([&]() {
            for (int i = 1; i <= 5; ++i) {
                // 缩小锁的作用范围
                {
                    scoped_lock<named_mutex> lock(mutex);
                    data->counter = i;
                    std::string msg = "Update #" + std::to_string(i);
                    strncpy(data->message, msg.c_str(), sizeof(data->message)-1);
                    data->message[sizeof(data->message)-1] = '\0';
                } // 锁在这里自动释放
                
                // 输出在锁外执行
                std::cout << "[Writer] Updated: counter=" << i 
                          << ", message=" << data->message << std::endl;
                std::cout.flush(); // 确保输出刷新
                
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            writer_done = true;
        });

        std::thread reader([&]() {
            while (!writer_done || data->counter < 5) {
                int current_counter;
                char current_message[256];
                
                // 快速获取快照
                {
                    scoped_lock<named_mutex> lock(mutex);
                    current_counter = data->counter;
                    strncpy(current_message, data->message, sizeof(current_message));
                } // 锁在这里自动释放
                
                if (current_counter > 0) {
                    std::cout << "[Reader] Current: counter=" << current_counter 
                              << ", message=" << current_message << std::endl;
                    std::cout.flush(); // 确保输出刷新
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });

        writer.join();
        reader.join();

        // 清理资源
        segment.destroy<SharedData>("SharedData");
        named_mutex::remove(MUTEX_NAME);
        shared_memory_object::remove(SHARED_MEMORY_NAME);
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        // 确保异常时清理资源
        try { named_mutex::remove(MUTEX_NAME); } catch(...) {}
        try { shared_memory_object::remove(SHARED_MEMORY_NAME); } catch(...) {}
        return EXIT_FAILURE;
    }

    std::cout << "Program finished successfully. Press Enter to exit...";
    std::cin.get(); // 等待用户输入，防止窗口关闭
    
    return EXIT_SUCCESS;
}