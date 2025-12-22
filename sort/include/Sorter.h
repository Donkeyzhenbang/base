#ifndef SORTER_H
#define SORTER_H

#include "SortStrategy.h"
#include <memory>
#include <vector>
#include <chrono>

class Sorter {
private:
    std::unique_ptr<SortStrategy> strategy_;
    
public:
    Sorter() = default;
    explicit Sorter(std::unique_ptr<SortStrategy> strategy);
    
    void setStrategy(std::unique_ptr<SortStrategy> strategy);
    void executeSort(std::vector<int>& data);
    void displayInfo() const;
    void displayData(const std::vector<int>& data, const std::string& label = "") const;
    
    // 性能测试
    double measurePerformance(std::vector<int>& data);
    
    static std::vector<int> generateRandomData(int size, int min = 1, int max = 1000);
    static std::vector<int> generateSortedData(int size);
    static std::vector<int> generateReverseSortedData(int size);
};

#endif