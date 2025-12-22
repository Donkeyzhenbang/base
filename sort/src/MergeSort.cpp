#include "MergeSort.h"
#include <vector>

// 归并 分割为每个单独的元素 在最后归并到时候递归排序
void MergeSort::sort(std::vector<int>& data) {
    if (data.size() <= 1) return;
    mergeSort(data, 0, data.size() - 1);
}

void MergeSort::mergeSort(std::vector<int>& data, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        
        // 递归排序左右两半
        mergeSort(data, left, mid);
        mergeSort(data, mid + 1, right);
        
        // 合并已排序的两半
        merge(data, left, mid, right);
    }
}

void MergeSort::merge(std::vector<int>& data, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    
    // 创建临时数组
    std::vector<int> leftArr(n1);
    std::vector<int> rightArr(n2);
    
    // 拷贝数据到临时数组
    for (int i = 0; i < n1; ++i) {
        leftArr[i] = data[left + i];
    }
    for (int j = 0; j < n2; ++j) {
        rightArr[j] = data[mid + 1 + j];
    }
    
    // 合并临时数组回原数组
    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (leftArr[i] <= rightArr[j]) {
            data[k] = leftArr[i];
            i++;
        } else {
            data[k] = rightArr[j];
            j++;
        }
        k++;
    }
    
    // 拷贝剩余元素
    while (i < n1) {
        data[k] = leftArr[i];
        i++;
        k++;
    }
    
    while (j < n2) {
        data[k] = rightArr[j];
        j++;
        k++;
    }
}

std::string MergeSort::getName() const {
    return "归并排序 (Merge Sort)";
}

std::string MergeSort::getTimeComplexity() const {
    return "O(n log n)";
}

std::string MergeSort::getSpaceComplexity() const {
    return "O(n)";
}