#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <assert.h> // 调试用

#define ARRAY_SIZE 1000000000

// 复杂计算函数
double heavy_computation(int value) {
    double temp = sin(value) + cos(value) - tan((value % 100 + 1) % 89); 
    temp = ((int)(temp * value) % 5) + sqrt(value % 50);
    return temp / (temp + 1);
}

// 不使用预取的版本
void process_data_no_prefetch(int *array, int size) {
    for (int i = 0; i < size; i++) {
        int idx = ((i * 37) % size + size) % size; // 修复: 确保 idx 非负
        int idx_next1 = ((idx + 1) % size + size) % size; // 修复: 确保 idx_next1 非负
        int idx_next2 = ((idx + 2) % size + size) % size; // 修复: 确保 idx_next2 非负

        // 检查数组索引合法性（仅调试阶段使用）
        assert(idx >= 0 && idx < size);
        assert(idx_next1 >= 0 && idx_next1 < size);
        assert(idx_next2 >= 0 && idx_next2 < size);

        double result = heavy_computation(array[idx]);
        array[idx] = (int)result + array[idx_next1] * 3 - array[idx_next2] % 7;
    }
}

// 使用预取的版本
void process_data_with_prefetch(int *array, int size) {
    for (int i = 0; i < size; i++) {
        int idx = ((i * 37) % size + size) % size; // 修复: 确保 idx 非负
        int idx_next1 = ((idx + 1) % size + size) % size; // 修复: 确保 idx_next1 非负
        int idx_next2 = ((idx + 2) % size + size) % size; // 修复: 确保 idx_next2 非负

        // 检查数组索引合法性（仅调试阶段使用）
        assert(idx >= 0 && idx < size);
        assert(idx_next1 >= 0 && idx_next1 < size);
        assert(idx_next2 >= 0 && idx_next2 < size);

        if (idx + 64 >= size) { 
            idx = size - 64; // 避免预取越界
        }
        __builtin_prefetch(&array[idx + 64], 0, 1);

        double result = heavy_computation(array[idx]);
        array[idx] = (int)result + array[idx_next1] * 3 - array[idx_next2] % 7;
    }
}


double measure_time(void (*func)(int *, int), int *array, int size) {
    clock_t start = clock();
    func(array, size);
    clock_t end = clock();
    return (double)(end - start) / CLOCKS_PER_SEC;
}

int main() {

    int *data = (int *)malloc(ARRAY_SIZE * sizeof(int));
    if (data == NULL) {
        printf("内存分配失败！\n");
        return 1;
    }


    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i;
    }

    printf("开始性能测试...\n");

    // 不使用预取指令
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i; 
    }
    double time_no_prefetch = measure_time(process_data_no_prefetch, data, ARRAY_SIZE);
    printf("不使用预取的运行时间: %.6f 秒\n", time_no_prefetch);

    // 使用预取指令
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i; 
    }
    double time_with_prefetch = measure_time(process_data_with_prefetch, data, ARRAY_SIZE);
    printf("使用预取的运行时间: %.6f 秒\n", time_with_prefetch);

    // 释放动态分配的内存
    free(data);

    return 0;
}
