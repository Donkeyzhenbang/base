#include "BubbleSort.h"
#include <iostream>

void BubbleSort::sort(std::vector<int>& data) {
    int n = data.size();
    for (int i = 0; i < n - 1; ++i) {
        bool swapped = false;
        for (int j = 0; j < n - i - 1; ++j) {
            if (data[j] > data[j + 1]) {
                std::swap(data[j], data[j + 1]);
                swapped = true;
            }
        }
        // 如果没有发生交换，说明已经有序
        if (!swapped) break;
    }
}

std::string BubbleSort::getName() const {
    return "冒泡排序 (Bubble Sort)";
}

std::string BubbleSort::getTimeComplexity() const {
    return "最好: O(n), 平均: O(n²), 最坏: O(n²)";
}

std::string BubbleSort::getSpaceComplexity() const {
    return "O(1)";
}