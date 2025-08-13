#include <stdio.h>
#include <sys/times.h>
#include <unistd.h>

#define ARRAY_SIZE 1000000

void process_data(int *array, int size) {
    for (int i = 0; i < size; i++) {
        if (i + 1 < size) {
            __builtin_prefetch(&array[i + 1], 0, 1); // 数据预取
        }
        array[i] *= 2;
    }
}

int main() {
    int data[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i;
    }

    struct tms start, end;
    clock_t start_time, end_time;
    long ticks = sysconf(_SC_CLK_TCK);

    start_time = times(&start); // 开始时间
    process_data(data, ARRAY_SIZE);
    end_time = times(&end); // 结束时间

    printf("用户态时间: %.6f 秒\n", (double)(end.tms_utime - start.tms_utime) / ticks);
    printf("内核态时间: %.6f 秒\n", (double)(end.tms_stime - start.tms_stime) / ticks);
    printf("总运行时间: %.6f 秒\n", (double)(end_time - start_time) / ticks);

    return 0;
}
