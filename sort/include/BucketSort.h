#ifndef BUCKETSORT_H
#define BUCKETSORT_H

#include "SortStrategy.h"

class BucketSort : public SortStrategy {
public:
    void sort(std::vector<int>& data) override;
    std::string getName() const override;
    std::string getTimeComplexity() const override;
    std::string getSpaceComplexity() const override;
    
private:
    void insertionSort(std::vector<int>& bucket);
};

#endif