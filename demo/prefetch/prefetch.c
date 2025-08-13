#include <stdio.h>
#include <time.h>

#define ARRAY_SIZE 100000000

void process_data(int *array, int size) {
    for (int i = 0; i < size; i++) {
        if (i + 1 < size) {
            __builtin_prefetch(&array[i + 1], 0, 1); // 预取下一元素
        }
        array[i] *= 2; // 数据处理
    }
}

int main() {
    int data[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i;
    }

    clock_t start = clock(); // 记录开始时间

    process_data(data, ARRAY_SIZE);

    clock_t end = clock(); // 记录结束时间

    double elapsed_time = (double)(end - start) / CLOCKS_PER_SEC; // 转换为秒
    printf("程序运行时间: %.6f 秒\n", elapsed_time);

    return 0;
}
