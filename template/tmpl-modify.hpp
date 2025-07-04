#pragma once

#include <type_traits>
#include <initializer_list>
#include <algorithm>
#include <stdexcept>

// 类型萃取 - 获取元素的基础类型
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

template<typename T>
class MyArray {
private:
    T* data = nullptr;
    size_t m_size = 0;

public:
    // 迭代器类型定义
    using iterator = T*;
    using const_iterator = const T*;

    // 构造函数
    MyArray() = default;
    
    explicit MyArray(size_t size) : 
        m_size(size), 
        data(size ? new T[size]() : nullptr) 
    {}
    
    // 初始化列表构造函数
    MyArray(std::initializer_list<T> list) try : 
        m_size(list.size()), 
        data(list.size() ? new T[list.size()] : nullptr) 
    {
        if (!data) return;
        
        size_t i = 0;
        if constexpr (std::is_pointer_v<T>) {
            // 对指针类型进行深拷贝
            for (const auto& elem : list) {
                data[i] = new element_type_t<T>(*elem);
                ++i;
            }
        } else {
            // 普通类型直接复制
            std::copy(list.begin(), list.end(), data);
        }
    } catch (...) {
        cleanup();
        throw;
    }
    
    // 拷贝构造函数
    MyArray(const MyArray& other) : 
        m_size(other.m_size), 
        data(other.m_size ? new T[other.m_size] : nullptr) 
    {
        if (!data) return;
        
        for (size_t i = 0; i < m_size; ++i) {
            if constexpr (std::is_pointer_v<T>) {
                data[i] = new element_type_t<T>(*other.data[i]);
            } else {
                data[i] = other.data[i];
            }
        }
    }
    
    // 移动构造函数
    MyArray(MyArray&& other) noexcept : 
        data(other.data), 
        m_size(other.m_size) 
    {
        other.data = nullptr;
        other.m_size = 0;
    }
    
    // 析构函数
    ~MyArray() {
        cleanup();
    }
    
    // 赋值运算符
    MyArray& operator=(const MyArray& other) {
        if (this == &other) return *this;
        
        MyArray tmp(other);
        swap(tmp);
        return *this;
    }
    
    MyArray& operator=(MyArray&& other) noexcept {
        if (this == &other) return *this;
        
        swap(other);
        return *this;
    }
    
    // 访问元素
    T& operator[](size_t index) { 
        return data[index]; 
    }
    
    const T& operator[](size_t index) const { 
        return data[index]; 
    }
    
    T& at(size_t index) {
        if (index >= m_size) throw std::out_of_range("Index out of range");
        return data[index];
    }
    
    const T& at(size_t index) const {
        if (index >= m_size) throw std::out_of_range("Index out of range");
        return data[index];
    }
    
    // 迭代器
    iterator begin() { return data; }
    iterator end() { return data + m_size; }
    
    const_iterator begin() const { return data; }
    const_iterator end() const { return data + m_size; }
    
    const_iterator cbegin() const { return data; }
    const_iterator cend() const { return data + m_size; }
    
    // 容量
    size_t size() const { return m_size; }
    bool empty() const { return m_size == 0; }
    
    // 交换
    void swap(MyArray& other) noexcept {
        using std::swap;
        swap(data, other.data);
        swap(m_size, other.m_size);
    }

private:
    void cleanup() {
        if (!data) return;
        
        if constexpr (std::is_pointer_v<T>) {
            // 释放每个指针指向的内存
            for (size_t i = 0; i < m_size; ++i) {
                if (data[i]) {
                    delete data[i];
                    data[i] = nullptr;
                }
            }
        }
        delete[] data;
        data = nullptr;
        m_size = 0;
    }
};

// 非成员swap函数
template <typename T>
void swap(MyArray<T>& lhs, MyArray<T>& rhs) noexcept {
    lhs.swap(rhs);
}