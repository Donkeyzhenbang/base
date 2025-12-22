#ifndef MERGESORT_H
#define MERGESORT_H

#include "SortStrategy.h"

class MergeSort : public SortStrategy {
public:
    void sort(std::vector<int>& data) override;
    std::string getName() const override;
    std::string getTimeComplexity() const override;
    std::string getSpaceComplexity() const override;
    
private:
    void mergeSort(std::vector<int>& data, int left, int right);
    void merge(std::vector<int>& data, int left, int mid, int right);
};

#endif