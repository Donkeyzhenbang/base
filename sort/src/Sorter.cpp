#include "Sorter.h"
#include <iostream>
#include <random>
#include <algorithm>

Sorter::Sorter(std::unique_ptr<SortStrategy> strategy) 
    : strategy_(std::move(strategy)) {}

void Sorter::setStrategy(std::unique_ptr<SortStrategy> strategy) {
    strategy_ = std::move(strategy);
}

void Sorter::executeSort(std::vector<int>& data) {
    if (!strategy_) {
        std::cout << "错误: 未设置排序策略!" << std::endl;
        return;
    }
    
    std::cout << "执行 " << strategy_->getName() << std::endl;
    strategy_->sort(data);
}

void Sorter::displayInfo() const {
    if (!strategy_) {
        std::cout << "错误: 未设置排序策略!" << std::endl;
        return;
    }
    
    std::cout << "算法: " << strategy_->getName() << std::endl;
    std::cout << "时间复杂度: " << strategy_->getTimeComplexity() << std::endl;
    std::cout << "空间复杂度: " << strategy_->getSpaceComplexity() << std::endl;
}

void Sorter::displayData(const std::vector<int>& data, const std::string& label) const {
    if (!label.empty()) {
        std::cout << label << ": ";
    }
    
    for (size_t i = 0; i < data.size(); ++i) {
        std::cout << data[i];
        if (i < data.size() - 1) {
            std::cout << " ";
        }
    }
    std::cout << std::endl;
}

double Sorter::measurePerformance(std::vector<int>& data) {
    if (!strategy_) return -1.0;
    
    auto start = std::chrono::high_resolution_clock::now();
    strategy_->sort(data);
    auto end = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double> duration = end - start;
    return duration.count();
}

std::vector<int> Sorter::generateRandomData(int size, int min, int max) {
    std::vector<int> data(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    
    for (int i = 0; i < size; ++i) {
        data[i] = dis(gen);
    }
    return data;
}

std::vector<int> Sorter::generateSortedData(int size) {
    std::vector<int> data(size);
    for (int i = 0; i < size; ++i) {
        data[i] = i + 1;
    }
    return data;
}

std::vector<int> Sorter::generateReverseSortedData(int size) {
    std::vector<int> data(size);
    for (int i = 0; i < size; ++i) {
        data[i] = size - i;
    }
    return data;
}