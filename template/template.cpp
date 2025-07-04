#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include "tmpl.hpp"


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
}