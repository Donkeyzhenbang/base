#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <functional>
#include "tmpl.hpp"
// #include "template.hpp"

namespace mystd
{
    using void_int_func_type = std::function<void(int&)>;
    template<typename iter_type, typename func_type >
    void for_each(iter_type first, iter_type last, func_type func = [](int& elem){
        ++elem;
    })
    {
        for(auto iter = first; iter != last; ++iter){
            func(*iter);
        }
    }

    template<typename T>
    class MyVector
    {
    public:
        template<typename T2>
        void output(const T2& elem)
        {
            std::cout << elem << std::endl;
        }
    };

} // namespace mystd


int main()
{
    int i1 = 10;
    int i2 = 20;
    int i3 = 30;
    int i4 = 40;
    std::initializer_list<int*> iList{ &i1, &i2, &i3, &i4};
    std::cout << "begin ： " <<  *(iList.begin()) << "\nend：  " << iList.end() << "\nsize : " << iList.size() << std::endl;

    MyArray<int*> arrayPi(iList);   //这里arrayPi也是迭代器指针
    for(unsigned i = 0; i < 4; i ++){
        std::cout << *arrayPi[i] << std::endl;
    }
    //void(*)(int&) 函数指针
    std::vector<int> ivec {1, 2, 3, 4, 5};
    mystd::for_each<std::vector<int>::iterator, void(*)(int&)>(ivec.begin(), ivec.end(), [](int& elem)
    {
        ++elem;
    });

    // 模板默认参数
    mystd::for_each<std::vector<int>::iterator, void(*)(int&)>(ivec.begin(), ivec.end());

    for(int elem : ivec){
        std::cout << elem << std::endl;
    }


}