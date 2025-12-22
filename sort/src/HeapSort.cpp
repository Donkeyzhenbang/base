#include "HeapSort.h"
// 把数组当成一棵完全二叉树，先构建最大堆，使最大值放在堆顶。
// 然后不断把堆顶（最大值）交换到数组末尾，再对剩余部分重新堆化。

//! 构建大顶堆，容器首部元素即为最大元素，每次把它放到数组最后其他保持堆化，这样即可实现堆排序

void HeapSort::sort(std::vector<int>& data) {
    int n = data.size();
    
    // 构建最大堆
    buildHeap(data, n);
    
    // 一个一个从堆顶取出元素
    for (int i = n - 1; i > 0; --i) {
        std::swap(data[0], data[i]);
        heapify(data, i, 0);
    }
}

void HeapSort::heapify(std::vector<int>& data, int n, int i) {
    int largest = i;        // 初始化最大值为根
    int left = 2 * i + 1;   // 左子节点
    int right = 2 * i + 2;  // 右子节点
    
    // 如果左子节点比根大
    if (left < n && data[left] > data[largest]) {
        largest = left;
    }
    
    // 如果右子节点比当前最大值大
    if (right < n && data[right] > data[largest]) {
        largest = right;
    }
    
    // 如果最大值不是根
    if (largest != i) {
        std::swap(data[i], data[largest]);
        // 递归地堆化受影响的子堆
        heapify(data, n, largest);
    }
}

void HeapSort::buildHeap(std::vector<int>& data, int n) {
    // 从最后一个非叶子节点开始构建堆
    for (int i = n / 2 - 1; i >= 0; --i) {
        heapify(data, n, i);
    }
}

std::string HeapSort::getName() const {
    return "堆排序 (Heap Sort)";
}

std::string HeapSort::getTimeComplexity() const {
    return "O(n log n)";
}

std::string HeapSort::getSpaceComplexity() const {
    return "O(1)";
}