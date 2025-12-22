#ifndef QUICKSORT_H
#define QUICKSORT_H

#include "SortStrategy.h"

class QuickSort : public SortStrategy {
public:
    void sort(std::vector<int>& data) override;
    std::string getName() const override;
    std::string getTimeComplexity() const override;
    std::string getSpaceComplexity() const override;
    
private:
    void quickSort(std::vector<int>& data, int low, int high);
    int partition(std::vector<int>& data, int low, int high);
};

#endif