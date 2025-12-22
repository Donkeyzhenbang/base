#include <iostream>
#include <memory>
#include <vector>
#include <iomanip>

#include "BubbleSort.h"
#include "BucketSort.h"
#include "HeapSort.h"
#include "MergeSort.h"
#include "QuickSort.h"
#include "Sorter.h"

void demonstrateAllAlgorithms() {
    std::cout << "=== 排序算法策略模式演示 ===" << std::endl << std::endl;
    
    // 测试数据
    std::vector<int> testData = {64, 34, 25, 12, 22, 11, 90, 88, 76, 50, 42, 33, 21, 19, 8};
    
    Sorter sorter;
    
    // 1. 冒泡排序演示
    std::cout << "1. 冒泡排序演示:" << std::endl;
    sorter.setStrategy(std::make_unique<BubbleSort>());
    sorter.displayInfo();
    auto data1 = testData;
    sorter.displayData(data1, "排序前");
    sorter.executeSort(data1);
    sorter.displayData(data1, "排序后");
    std::cout << std::endl;
    
    // 2. 桶排序演示
    std::cout << "2. 桶排序演示:" << std::endl;
    sorter.setStrategy(std::make_unique<BucketSort>());
    sorter.displayInfo();
    auto data2 = testData;
    sorter.displayData(data2, "排序前");
    sorter.executeSort(data2);
    sorter.displayData(data2, "排序后");
    std::cout << std::endl;
    
    // 3. 堆排序演示
    std::cout << "3. 堆排序演示:" << std::endl;
    sorter.setStrategy(std::make_unique<HeapSort>());
    sorter.displayInfo();
    auto data3 = testData;
    sorter.displayData(data3, "排序前");
    sorter.executeSort(data3);
    sorter.displayData(data3, "排序后");
    std::cout << std::endl;
    
    // 4. 归并排序演示
    std::cout << "4. 归并排序演示:" << std::endl;
    sorter.setStrategy(std::make_unique<MergeSort>());
    sorter.displayInfo();
    auto data4 = testData;
    sorter.displayData(data4, "排序前");
    sorter.executeSort(data4);
    sorter.displayData(data4, "排序后");
    std::cout << std::endl;
    
    // 5. 快速排序演示
    std::cout << "5. 快速排序演示:" << std::endl;
    sorter.setStrategy(std::make_unique<QuickSort>());
    sorter.displayInfo();
    auto data5 = testData;
    sorter.displayData(data5, "排序前");
    sorter.executeSort(data5);
    sorter.displayData(data5, "排序后");
    std::cout << std::endl;
}

void performanceComparison() {
    std::cout << "=== 性能比较测试 ===" << std::endl << std::endl;
    
    // 生成测试数据
    const int dataSize = 1000;
    auto randomData = Sorter::generateRandomData(dataSize, 1, 10000);
    auto sortedData = Sorter::generateSortedData(dataSize);
    auto reverseData = Sorter::generateReverseSortedData(dataSize);
    
    Sorter sorter;
    
    // 测试不同算法在随机数据上的性能
    std::cout << "随机数据性能测试 (" << dataSize << " 个元素):" << std::endl;
    std::cout << std::fixed << std::setprecision(6);
    
    // 冒泡排序
    sorter.setStrategy(std::make_unique<BubbleSort>());
    auto data1 = randomData;
    double time1 = sorter.measurePerformance(data1);
    std::cout << "冒泡排序: " << time1 << " 秒" << std::endl;
    
    // 桶排序
    sorter.setStrategy(std::make_unique<BucketSort>());
    auto data2 = randomData;
    double time2 = sorter.measurePerformance(data2);
    std::cout << "桶排序: " << time2 << " 秒" << std::endl;
    
    // 堆排序
    sorter.setStrategy(std::make_unique<HeapSort>());
    auto data3 = randomData;
    double time3 = sorter.measurePerformance(data3);
    std::cout << "堆排序: " << time3 << " 秒" << std::endl;
    
    // 归并排序
    sorter.setStrategy(std::make_unique<MergeSort>());
    auto data4 = randomData;
    double time4 = sorter.measurePerformance(data4);
    std::cout << "归并排序: " << time4 << " 秒" << std::endl;
    
    // 快速排序
    sorter.setStrategy(std::make_unique<QuickSort>());
    auto data5 = randomData;
    double time5 = sorter.measurePerformance(data5);
    std::cout << "快速排序: " << time5 << " 秒" << std::endl;
    
    std::cout << std::endl;
}

void interactiveDemo() {
    std::cout << "=== 交互式演示 ===" << std::endl << std::endl;
    
    Sorter sorter;
    std::vector<int> data;
    
    while (true) {
        std::cout << "选择操作:" << std::endl;
        std::cout << "1. 输入数据" << std::endl;
        std::cout << "2. 使用冒泡排序" << std::endl;
        std::cout << "3. 使用桶排序" << std::endl;
        std::cout << "4. 使用堆排序" << std::endl;
        std::cout << "5. 使用归并排序" << std::endl;
        std::cout << "6. 使用快速排序" << std::endl;
        std::cout << "7. 显示当前数据" << std::endl;
        std::cout << "0. 退出" << std::endl;
        std::cout << "请选择: ";
        
        int choice;
        std::cin >> choice;
        
        if (choice == 0) break;
        
        switch (choice) {
            case 1: {
                std::cout << "输入数据数量: ";
                int n;
                std::cin >> n;
                data.resize(n);
                std::cout << "输入 " << n << " 个整数: ";
                for (int i = 0; i < n; ++i) {
                    std::cin >> data[i];
                }
                break;
            }
            case 2:
                sorter.setStrategy(std::make_unique<BubbleSort>());
                break;
            case 3:
                sorter.setStrategy(std::make_unique<BucketSort>());
                break;
            case 4:
                sorter.setStrategy(std::make_unique<HeapSort>());
                break;
            case 5:
                sorter.setStrategy(std::make_unique<MergeSort>());
                break;
            case 6:
                sorter.setStrategy(std::make_unique<QuickSort>());
                break;
            case 7:
                sorter.displayData(data, "当前数据");
                break;
            default:
                std::cout << "无效选择!" << std::endl;
                continue;
        }
        
        if (choice >= 2 && choice <= 6) {
            if (data.empty()) {
                std::cout << "请先输入数据!" << std::endl;
            } else {
                sorter.displayInfo();
                sorter.displayData(data, "排序前");
                sorter.executeSort(data);
                sorter.displayData(data, "排序后");
            }
        }
        
        std::cout << std::endl;
    }
}

int main() {
    // 演示所有算法
    demonstrateAllAlgorithms();
    
    // 性能比较
    performanceComparison();
    
    // 交互式演示
    interactiveDemo();
    
    return 0;
}