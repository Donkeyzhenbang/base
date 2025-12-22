#ifndef BUBBLESORT_H
#define BUBBLESORT_H

#include "SortStrategy.h"

class BubbleSort : public SortStrategy {
public:
    void sort(std::vector<int>& data) override;
    std::string getName() const override;
    std::string getTimeComplexity() const override;
    std::string getSpaceComplexity() const override;
};

#endif