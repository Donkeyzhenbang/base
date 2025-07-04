#include <iostream>
#include <type_traits>

template <typename T>
struct element_type {
    using type = T;
};

template <typename T>
struct element_type<T*> {
    using type = typename element_type<T>::type;
};

template <typename T>
using element_type_t = typename element_type<T>::type;


template <typename T>
class Vector {
private:
    T* data;
    size_t size_;
    size_t capacity_;

public:
    Vector() : data(nullptr), size_(0), capacity_(0) {}
    
    Vector(size_t n, const T& val = T()) : size_(n), capacity_(n) {
        data = new T[n];
        for (size_t i = 0; i < n; ++i) data[i] = val;
    }
    Vector(const std::initializer_list<T>& list){
        size_ = list.size();
        capacity_ = list.size();
        if(list.size()){
            size_t count = 0;
            data = new T[list.size()]();
            if constexpr (std::is_pointer<T>::value)
            {
                //  T为int时 此时条件为false 但是编译器还是会选择编译这部分代码
                //  编译器报告 unary '*' 错误的原因。要解决这个问题，必须确保当 T` 不是指针类型时，相关的代码不会被实例化或编译。
                for(const auto& elem : list){
                    data[count++] = new element_type_t<T>(*elem);
                }
            }
            else 
            {
                for(const auto& elem : list){
                    data[count ++] = elem;
                }
            }
        }
        else{
            data = nullptr;
        }
    }
    
    ~Vector() { delete[] data; }
    
    // 拷贝构造
    Vector(const Vector& other) : size_(other.size_), capacity_(other.capacity_) {
        data = new T[capacity_];
        for (size_t i = 0; i < size_; ++i) data[i] = other.data[i];
    }
    
    // 拷贝赋值（copy-and-swap）
    Vector& operator=(Vector other) {
        swap(other);
        return *this;
    }
    
    void swap(Vector& other) noexcept {
        std::swap(data, other.data);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }
    
    void push_back(const T& val) {
        if (size_ >= capacity_) {
            size_t new_cap = (capacity_ == 0) ? 1 : capacity_ * 2;
            reserve(new_cap);
        }
        data[size_++] = val;
    }
    
    void reserve(size_t new_cap) {
        if (new_cap <= capacity_) return;
        T* new_data = new T[new_cap];
        for (size_t i = 0; i < size_; ++i) new_data[i] = data[i];
        delete[] data;
        data = new_data;
        capacity_ = new_cap;
    }
    
    T& operator[](size_t index) { return data[index]; }
    size_t size() const { return size_; }
    size_t capacity() const { return capacity_; }
};

int main()
{
    int a[] = {1, 2, 3, 4, 5};
    Vector<int> vec(5, 3);
    Vector<int> vec2{3, 5, 7, 8};
    std::cout << "Capcity : " << vec2.capacity() << std::endl;
    for(int i = 0 ; i < vec.size(); i ++){
        std::cout << "val : " <<  vec[i] << " \n";
    }

    return 0;
}