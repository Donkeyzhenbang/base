#include <iostream>
#include <coroutine>
#include <thread>
#include <future>// 定义一个协程类型
#include "unistd.h"
struct MyCoroutine {
    struct promise_type;  // 前置声明
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type {
        MyCoroutine get_return_object() {
            return MyCoroutine(handle_type::from_promise(*this));
        }

        std::suspend_always initial_suspend() {
            std::cout << "coroutine initial\n";
            return {};
        }

        std::suspend_always final_suspend() noexcept {
            return {};
        }

        void return_void() {}

        void unhandled_exception() {
            std::exit(1);
        }
    };

    handle_type h;
    MyCoroutine(handle_type h) : h(h) {}
    ~MyCoroutine() { h.destroy(); }
};

MyCoroutine example_coroutine() {
    std::cout << "Coroutine started!\n";
    co_await std::suspend_always{};  // 挂起协程，等待恢复
    std::cout << "Coroutine resumed!\n";
}

int main() {
    std::cout << "Main function start!\n";
    auto coro = example_coroutine();  // 启动协程
    sleep(5);
    std::cout << "Main function after coroutine launch!\n";
    coro.h.resume();  // 恢复协程
    return 0;
}


