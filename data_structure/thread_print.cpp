#include <iostream>
#include <condition_variable>
#include <thread>
#include <mutex>

int main() {
    std::mutex m;
    std::condition_variable cv;
    int state = 0; // 0 a 1 b 2 c
    int N = 100;


    auto worker = [&](int id, char ch) {
        for(int i = 0; i < N; i ++){
            std::unique_lock<std::mutex> lk(m);
            cv.wait(lk, [&]{ return state == id;});
            std::cout << ch;
            state = (state + 1) % 3;
            lk.unlock();
            cv.notify_all();
        }
    };

    std::thread t1(worker, 0, 'a');
    std::thread t2(worker, 1, 'b');
    std::thread t3(worker, 2, 'c');

    t1.join();
    t2.join();
    t3.join();

    std::cout << std::endl;
    return 0;

}