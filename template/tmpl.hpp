#pragma once

#include <type_traits>

template <typename T>
struct get_type
{
	using type = T;
};

//如果是指针即可获得其指向的类型，相当于这里只对指针类型做处理 模板特化
template <typename T>
struct get_type<T*>
{
	using type = T;
};

//! 注意这里只释放了指针 并没有释放指针指向的内存 会存在内存泄漏 使用vector或其他显式的内存释放策略
//! 这里已经修复
template<typename T>
class MyArray
{
	using iterator = T*;
	using const_iterator = const T*;
public:
	MyArray(size_t count) : m_size(count){};
	~MyArray();
	MyArray(const std::initializer_list<T>& list);
	MyArray(std::initializer_list<T>&& list);
	iterator begin() const;
	
	const_iterator cbegin() const;

	T& operator [](unsigned count) const
	{
		return data[count];
	}

private:
	T* data;
	size_t m_size;
	// std::vector<T> data;
};

template <typename T>
MyArray<T>::~MyArray()
{
	// if(data)
	// {
	// 	delete[] data;
	// }
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


template <typename T>
typename MyArray<T>::iterator MyArray<T>::begin() const
{
	return data;
}

template <typename T>
typename MyArray<T>::const_iterator MyArray<T>::cbegin() const
{
	return data;
}

template <typename T>
MyArray<T>::MyArray(const std::initializer_list<T>& list)
{//这里需要避免指针浅复制 ： 1.模板特化 2.traits技术
	m_size = list.size();
	if(list.size())
	{
		unsigned count = 0;
		data = new T[list.size()]();
		if(std::is_pointer<T>::value)
		{
            for(const auto& elem : list)
			{
			    data[count++] = new typename get_type<T>::type(*elem);//typename表示新的类型
			}

		}
		else
		{
			for(const auto& elem : list)
			{
				data[count++] = elem;
			}
		}
	}
	else
	{
		data = nullptr;
	}
}

template <typename T>
MyArray<T>::MyArray(std::initializer_list<T>&& list)
{   //对于右值引用，如果是指针类型，那么就会移动语义，转移权限，也是正确的
	if(list.size())
	{
		unsigned count = 0;
		data = new T[list.size()]();
        for(const auto& elem : list)
        {
            data[count++] = elem;
        }
	}
	else
	{
		data = nullptr;
	}
}