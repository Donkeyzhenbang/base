#ifndef SORTSTRATEGY_H
#define SORTSTRATEGY_H

#include <vector>
#include <string>

class SortStrategy {
public:
    virtual ~SortStrategy() = default;
    virtual void sort(std::vector<int>& data) = 0;
    virtual std::string getName() const = 0;
    virtual std::string getTimeComplexity() const = 0;
    virtual std::string getSpaceComplexity() const = 0;
};

#endif