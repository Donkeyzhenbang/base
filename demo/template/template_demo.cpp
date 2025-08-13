#include <iostream>
#include "template_demo.h"

template<typename T> T A<T>::add(T a, T b){
    return a + b;
}

int main()
{
    A<int> tmp;
    std::cout << tmp.add(2, 3.2) << std::endl;
    return 0;
}