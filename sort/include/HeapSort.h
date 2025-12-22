#ifndef HEAPSORT_H
#define HEAPSORT_H

#include "SortStrategy.h"

class HeapSort : public SortStrategy {
public:
    void sort(std::vector<int>& data) override;
    std::string getName() const override;
    std::string getTimeComplexity() const override;
    std::string getSpaceComplexity() const override;
    
private:
    void heapify(std::vector<int>& data, int n, int i);
    void buildHeap(std::vector<int>& data, int n);
};

#endif