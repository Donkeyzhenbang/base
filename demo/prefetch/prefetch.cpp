#include <iostream>
#include <vector>
#include <chrono>

#define ARRAY_SIZE 1000000

void process_data(std::vector<int> &array) {
    for (size_t i = 0; i < array.size(); i++) {
        if (i + 1 < array.size()) {
            __builtin_prefetch(&array[i + 1], 0, 1); // 数据预取
        }
        array[i] *= 2;
    }
}

int main() {
    std::vector<int> data(ARRAY_SIZE);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i;
    }

    auto start = std::chrono::high_resolution_clock::now(); // 开始时间

    process_data(data);

    auto end = std::chrono::high_resolution_clock::now(); // 结束时间

    std::chrono::duration<double> elapsed = end - start;
    std::cout << "程序运行时间: " << elapsed.count() << " 秒" << std::endl;

    return 0;
}
