#pragma once

#include <type_traits>
#include <initializer_list>
#include <stdexcept>

template <typename T>
struct get_type {
    using type = T;
};

template <typename T>
struct get_type<T*> {
    using type = T;
};

// 条件编译辅助结构
template<bool is_pointer, typename T>
struct ArrayInitializer;

template<typename T>
struct ArrayInitializer<false, T> {
    static void init(T* data, std::initializer_list<T> list) {
        size_t count = 0;
        for (const auto& elem : list) {
            data[count++] = elem;
        }
    }
};

template<typename T>
struct ArrayInitializer<true, T> {
    static void init(T* data, std::initializer_list<T> list) {
        size_t count = 0;
        for (const auto& elem : list) {
            data[count++] = new typename get_type<T>::type(*elem);
        }
    }
};

template<typename T>
class MyArray {
    using iterator = T*;
    using const_iterator = const T*;
public:
    MyArray(size_t count) : m_size(count), data(count ? new T[count]() : nullptr) {}
    
    MyArray(const std::initializer_list<T>& list) {
        if (list.size()) {
            m_size = list.size();
            data = new T[list.size()]();
            ArrayInitializer<std::is_pointer<T>::value, T>::init(data, list);
        } else {
            data = nullptr;
            m_size = 0;
        }
    }
    
    ~MyArray() {
        if (data) {
            if (std::is_pointer<T>::value) {
                for (size_t i = 0; i < m_size; ++i) {
                    if (data[i]) {
                        delete data[i];
                    }
                }
            }
            delete[] data;
        }
    }
    
    // 禁用拷贝
    MyArray(const MyArray&) = delete;
    MyArray& operator=(const MyArray&) = delete;
    
    iterator begin() { return data; }
    iterator end() { return data + m_size; }
    
    const_iterator begin() const { return data; }
    const_iterator end() const { return data + m_size; }
    
    const_iterator cbegin() const { return data; }
    const_iterator cend() const { return data + m_size; }
    
    T& operator[](size_t count) {
        if (count >= m_size) throw std::out_of_range("Index out of range");
        return data[count];
    }
    
    const T& operator[](size_t count) const {
        if (count >= m_size) throw std::out_of_range("Index out of range");
        return data[count];
    }
    
    size_t size() const { return m_size; }

private:
    T* data;
    size_t m_size;
};