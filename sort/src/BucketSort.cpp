#include "BucketSort.h"
#include <algorithm>
#include <vector>

void BucketSort::sort(std::vector<int>& data) {
    if (data.empty()) return;
    
    // 找到最大值和最小值
    int minVal = *std::min_element(data.begin(), data.end());
    int maxVal = *std::max_element(data.begin(), data.end());
    
    // 创建桶
    int bucketCount = data.size() / 2 + 1;
    if (bucketCount <= 0) bucketCount = 1;
    
    std::vector<std::vector<int>> buckets(bucketCount);
    
    // 将元素分配到桶中
    for (int num : data) {
        int bucketIndex = (num - minVal) * (bucketCount - 1) / (maxVal - minVal);
        buckets[bucketIndex].push_back(num);
    }
    
    // 对每个桶进行排序
    for (auto& bucket : buckets) {
        insertionSort(bucket);
    }
    
    // 合并桶
    int index = 0;
    for (const auto& bucket : buckets) {
        for (int num : bucket) {
            data[index++] = num;
        }
    }
}

void BucketSort::insertionSort(std::vector<int>& bucket) {
    for (int i = 1; i < bucket.size(); ++i) {
        int key = bucket[i];
        int j = i - 1;
        
        while (j >= 0 && bucket[j] > key) {
            bucket[j + 1] = bucket[j];
            j--;
        }
        bucket[j + 1] = key;
    }
}

std::string BucketSort::getName() const {
    return "桶排序 (Bucket Sort)";
}

std::string BucketSort::getTimeComplexity() const {
    return "最好: O(n+k), 平均: O(n+k), 最坏: O(n²)";
}

std::string BucketSort::getSpaceComplexity() const {
    return "O(n+k)";
}