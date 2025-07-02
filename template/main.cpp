#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
 
template <typename T>
class Matrix{
private:
    size_t rows_, cols_;
    std::unique_ptr<T[]> data_;
public:
    Matrix(size_t rows, size_t cols) 
        : rows_(rows), cols_(col), data(std::make_unique<T[]>(rows * cols))
        {
            std::cout << "构造矩阵" << rows_ << "x" << cols_ << std::endl;
        }
    Matirx
};

int main()
{

    return 0;
}