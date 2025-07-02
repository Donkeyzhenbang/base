#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>

// 类模板：泛型矩阵
template <typename T>
class Matrix {
private:
    size_t rows_, cols_;
    std::unique_ptr<T[]> data_;  // 使用unique_ptr管理动态数组

public:
    // 基础构造函数
    Matrix(size_t rows, size_t cols) 
        : rows_(rows), cols_(cols), data_(std::make_unique<T[]>(rows * cols)) 
    {
        std::cout << "构造 " << rows_ << "x" << cols_ << " 矩阵\n";
    }

    // 初始化列表构造函数
    Matrix(std::initializer_list<std::initializer_list<T>> init) 
        : rows_(init.size()), cols_(init.begin()->size()), 
          data_(std::make_unique<T[]>(rows_ * cols_)) 
    {
        std::cout << "列表初始化 " << rows_ << "x" << cols_ << " 矩阵\n";
        size_t i = 0;
        for (const auto& row : init) {
            std::copy(row.begin(), row.end(), &data_[i * cols_]);
            i++;
        }
    }

    // 拷贝构造函数
    Matrix(const Matrix& other) 
        : rows_(other.rows_), cols_(other.cols_), 
          data_(std::make_unique<T[]>(rows_ * cols_)) 
    {
        std::copy(other.data_.get(), other.data_.get() + rows_ * cols_, data_.get());
        std::cout << "拷贝构造矩阵\n";
    }

    // 移动构造函数
    Matrix(Matrix&& other) noexcept 
        : rows_(other.rows_), cols_(other.cols_), data_(std::move(other.data_)) 
    {
        other.rows_ = 0;
        other.cols_ = 0;
        std::cout << "移动构造矩阵\n";
    }

    // 运算符重载：矩阵加法
    Matrix operator+(const Matrix& rhs) const {
        if (rows_ != rhs.rows_ || cols_ != rhs.cols_) {
            throw std::invalid_argument("矩阵维度不匹配");
        }
        
        Matrix result(rows_, cols_);
        for (size_t i = 0; i < rows_ * cols_; ++i) {
            result.data_[i] = data_[i] + rhs.data_[i];
        }
        return result;
    }

    // 运算符重载：标量乘法
    Matrix operator*(T scalar) const {
        Matrix result(rows_, cols_);
        for (size_t i = 0; i < rows_ * cols_; ++i) {
            result.data_[i] = data_[i] * scalar;
        }
        return result;
    }

    // 运算符重载：下标访问
    T& operator()(size_t row, size_t col) {
        return data_[row * cols_ + col];
    }

    const T& operator()(size_t row, size_t col) const {
        return data_[row * cols_ + col];
    }

    // 运算符重载：输出流
    friend std::ostream& operator<<(std::ostream& os, const Matrix& mat) {
        for (size_t i = 0; i < mat.rows_; ++i) {
            for (size_t j = 0; j < mat.cols_; ++j) {
                os << mat(i, j) << " ";
            }
            os << "\n";
        }
        return os;
    }

    // 获取行数
    size_t rows() const { return rows_; }
    
    // 获取列数
    size_t cols() const { return cols_; }

    // 工厂方法：创建共享指针管理的矩阵
    static std::shared_ptr<Matrix> createShared(size_t rows, size_t cols) {
        return std::make_shared<Matrix>(rows, cols);
    }

    ~Matrix() {
        if (rows_ > 0 || cols_ > 0) {
            std::cout << "销毁 " << rows_ << "x" << cols_ << " 矩阵\n";
        }
    }
};

// 函数模板：矩阵点积
template <typename T>
std::unique_ptr<Matrix<T>> dotProduct(const Matrix<T>& a, const Matrix<T>& b) {
    if (a.cols() != b.rows()) {
        throw std::invalid_argument("矩阵维度不匹配");
    }
    
    auto result = std::make_unique<Matrix<T>>(a.rows(), b.cols());
    for (size_t i = 0; i < a.rows(); ++i) {
        for (size_t j = 0; j < b.cols(); ++j) {
            T sum = 0;
            for (size_t k = 0; k < a.cols(); ++k) {
                sum += a(i, k) * b(k, j);
            }
            (*result)(i, j) = sum;
        }
    }
    return result;
}

// 函数模板：打印智能指针管理的矩阵
template <typename T>
void printMatrix(const std::shared_ptr<Matrix<T>>& mat) {
    if (mat) {
        std::cout << "共享指针矩阵:\n" << *mat;
    } else {
        std::cout << "空矩阵指针\n";
    }
}

int main() {
    // 1. 使用unique_ptr管理矩阵
    auto mat1 = std::make_unique<Matrix<int>>(2, 2);
    mat1->operator()(0, 0) = 1; // 使用运算符重载
    (*mat1)(0, 1) = 2;          // 另一种调用方式
    (*mat1)(1, 0) = 3;
    (*mat1)(1, 1) = 4;
    
    std::cout << "\nmat1:\n" << *mat1;

    // 2. 使用初始化列表
    Matrix<int> mat2 = {
        {5, 6},
        {7, 8}
    };
    std::cout << "\nmat2:\n" << mat2;

    // 3. 运算符重载演示
    auto mat3 = *mat1 + mat2;
    std::cout << "\nmat1 + mat2:\n" << mat3;
    
    auto mat4 = mat3 * 2;
    std::cout << "\nmat3 * 2:\n" << mat4;

    // 4. 使用shared_ptr管理矩阵
    auto sharedMat1 = Matrix<double>::createShared(2, 3);
    (*sharedMat1)(0, 0) = 1.1; (*sharedMat1)(0, 1) = 2.2; (*sharedMat1)(0, 2) = 3.3;
    (*sharedMat1)(1, 0) = 4.4; (*sharedMat1)(1, 1) = 5.5; (*sharedMat1)(1, 2) = 6.6;
    
    auto sharedMat2 = std::make_shared<Matrix<double>>(Matrix<double>{
        {1.0, 2.0},
        {3.0, 4.0},
        {5.0, 6.0}
    });
    
    // 5. 函数模板使用
    auto product = dotProduct(*sharedMat1, *sharedMat2);
    std::cout << "\n点积结果:\n" << *product;

    // 6. 共享所有权演示
    {
        auto sharedCopy = sharedMat1;
        std::cout << "\n共享拷贝使用计数: " << sharedCopy.use_count() << "\n";
        (*sharedCopy)(0, 0) = 99.9; // 修改会影响原始对象
    }
    std::cout << "原始共享矩阵:\n" << *sharedMat1;

    // 7. 移动语义演示
    Matrix<double> tempMat(2, 2);
    tempMat(0, 0) = 1.5; tempMat(0, 1) = 2.5;
    tempMat(1, 0) = 3.5; tempMat(1, 1) = 4.5;
    
    auto movedMat = std::make_unique<Matrix<double>>(std::move(tempMat));
    std::cout << "\n移动后的矩阵:\n" << *movedMat;
    std::cout << "原始矩阵尺寸: " << tempMat.rows() << "x" << tempMat.cols() << "\n";

    return 0;
}