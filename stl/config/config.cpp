/**
 * 多示例选择编译
 * 编译命令：
 *   g++ -DLEC1 main.cpp -o main && ./main
 *   g++ -DLEC2 main.cpp -o main && ./main
 *   g++ -DLEC3 main.cpp -o main && ./main
 */

/**
 * lec1:in-class static constant integer initialization
 */
#include <iostream>
using namespace std;
//! 编写代码看着舒服 使用注意注释
// #define LEC1
// #define LEC2
// #define LEC3

#ifdef LEC1
template <typename T>
class testClass{
public:
    static const int _datai = 5;
    static const long _datal = 3L;
    static const char _datac = 'c';

};
int main()
{
    cout << testClass<int>::_datai << endl;
    cout << testClass<int>::_datal << endl;
    cout << testClass<int>::_datac << endl;
}
#endif

/**
 * lec2 : increment、decrement、dereference
 */
#ifdef LEC2
#include <iostream>
using namespace std;

class INT{
friend ostream& operator<<(ostream& os, const INT& i);
public:
    INT(int i) : m_i(i) {};
    // prefix increment and then fetch
    INT& operator++(){
        ++ (this->m_i);
        return *this;
    }
    // postfix fetch and then increment
    const INT operator++(int){
        INT temp = *this;
        ++(*this);
        return temp;
    }

    INT& operator--(){
        --(this->m_i);
        return *this;
    }
    const INT operator--(int){
        INT temp = *this;
        --(*this);
        return temp;
    }
    int& operator*() const
    {
        return (int&)m_i;
    }
private:
    int m_i;
};

ostream& operator<<(ostream& os, const INT& i){
    os<< '[' << i.m_i << ']' ;
    return os;
}

int main()
{
    INT I(5);
    cout << I++;
    cout << ++I;
    cout << I--;
    cout << --I;
    cout << *I; //解引用
}
#endif

// iterator last会到末尾下一个 off by one偏移一格
template<class InputIterator, class T>
InputIterator find(InputIterator first, InputIterator last, const T& value)
{
    while(first != last && *first != value) ++ first;
    return first;
}

template<class InputIterator, class Function>
Function for_each(InputIterator first, InputIterator last, Function f){
    for(;first != last; + first)
        f(*first);
    return f;
}

/**
 * lec2 : function call
 */
#ifdef LEC3
#include <cstdlib>
#include <iostream>
using namespace std;
int fcmp(const void* elem1, const void* elem2);

int main()
{
    int ia[10] = {32,92,67,58,10,4,25,52,59,54};
    for(int i = 0; i < 10; i ++)
        cout << ia[i] << " ";
    cout << endl;
    qsort(ia, sizeof(ia) / sizeof(int), sizeof(int), fcmp);
    for(int i = 0; i < 10; i ++)
        cout << ia[i] << " ";
}

int fcmp(const void* elem1, const void* elem2)
{
    const int* i1 = (const int*)elem1;
    const int* i2 = (const int*)elem2;
    if(*i1 < *i2)
        return -1;
    else if(*i1 == *i2)
        return 0;
    else if(*i1 > *i2)
        return 1;
}
#endif



//============= 默认：无宏定义时 =============
#ifndef LEC1
#ifndef LEC2
#ifndef LEC3
int main() {
    cout << "请使用 -D 参数指定要编译的示例：" << endl;
    cout << "  g++ -DLEC1 main.cpp -o main" << endl;
    cout << "  g++ -DLEC2 main.cpp -o main" << endl;
    cout << "  g++ -DLEC3 main.cpp -o main" << endl;
    return 0;
}
#endif // LEC3
#endif // LEC2
#endif // LEC1