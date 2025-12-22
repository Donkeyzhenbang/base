#include "QuickSort.h"
#include <algorithm>

void QuickSort::sort(std::vector<int>& data){
    if(data.empty()) return ;
    quickSort(data, 0, data.size() - 1);
}

void QuickSort::quickSort(std::vector<int>& data, int low, int high){
    if(low < high){
        int pi = partition(data, low, high);
        quickSort(data, low, pi - 1);
        quickSort(data, pi + 1, high)
    }
}

int QuickSort::partition(std::vector<int>& data, int low, int high){
    int pivot = data[high];
    int i = low - 1;
    for(int j = low; j < high; ++j){
        if(data[j] <= pivot){
            i ++;
            std::swap(data[i], data[j]);
        }
    }
    std::swap(data[i + 1], data[high]);
    return i + 1;

}

std::string QuickSort::getName() const {
    return "快速排序 (Quick Sort)";
}

std::string QuickSort::getTimeComplexity() const {
    return "最好: O(n log n), 平均: O(n log n), 最坏: O(n²)";
}

std::string QuickSort::getSpaceComplexity() const {
    return "O(log n)";
}