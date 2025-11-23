#include <iostream>
#include <string>
#include <chrono>
#include <vector>

template <typename T>
class LazySingleton {
private:
    LazySingleton() = default;
    ~LazySingleton() = default;
    LazySingleton(const LazySingleton&) = delete;
    LazySingleton& operator=(const LazySingleton&) = delete;
public:
    static T& getInstance() {
        // C++11保证局部静态变量初始化是线程安全的
        static T instance;
        return instance;
    }

};


//! 懒汉式 : 程序第一次被使用时才初始化，需要注意线程安全 Meyer‘s C++11保证线程安全
template <typename T>
class MeyersSingleton {
private:
    MeyersSingleton() = default;
    ~MeyersSingleton() = default;
    MeyersSingleton(const MeyersSingleton&) = delete;
    MeyersSingleton& operator=(const MeyersSingleton&) = delete;
public:
    static T& getInstance() {
        // C++11保证局部静态变量初始化是线程安全的
        static T instance;
        return instance;
    }

};

class Logger {
private:
    std::vector<std::string> logs_;
    
public:
    Logger() {
        std::cout << "Logger constructed (Lazy)" << std::endl;
    }
    
    ~Logger() {
        std::cout << "Logger destroyed" << std::endl;
    }
    
    void log(const std::string& message) {
        std::string log_entry = "[" + getCurrentTime() + "] " + message;
        logs_.push_back(log_entry);
        std::cout << log_entry << std::endl;
    }
    
    void showAllLogs() {
        std::cout << "=== All Logs ===" << std::endl;
        for (const auto& log : logs_) {
            std::cout << log << std::endl;
        }
    }
    
private:
    std::string getCurrentTime() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::string time_str = std::ctime(&time_t);
        time_str.pop_back(); // 移除换行符
        return time_str;
    }
};


void testLazySingleton()
{
    std::cout << "\n=== Testing Lazy Singleton ===" << std::endl;

    // Logger* logger1 = LazySingleton<Logger>::getInstance();
    // Logger* logger2 = LazySingleton<Logger>::getInstance();

    // // 验证是同一个实例
    // std::cout << "logger1 address: " << logger1 << std::endl;
    // std::cout << "logger2 address: " << logger2 << std::endl;
    // std::cout << "Same instance: " << (logger1 == logger2) << std::endl;

    // // 使用单例
    // logger1->log("Application started");
    // logger1->log("User logged in");
    // logger1->log("Database connection established");


    // // 使用Meyer's Singleton
    std::cout << "\n=== Testing Meyer's Singleton ===" << std::endl;
    MeyersSingleton<Logger>::getInstance().log("Using Meyer's singleton first");
    MeyersSingleton<Logger>::getInstance().log("Using Meyer's singleton second");
    MeyersSingleton<Logger>::getInstance().showAllLogs();


}

int main()
{
    testLazySingleton();
    return 0;
}