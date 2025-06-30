#include <iostream>
#include <vector>
#include <string>

class Test{

    friend std::ostream& operator<< (std::ostream& os, const Test& test);
    friend std::istream& operator>> (std::istream& is, Test& test);
public:
    void operator++ () //前缀自增
    {
        ++count;
    }

    unsigned get_cnt() { return count;}
private:
    unsigned count = 0;
    std::string name;
};

std::ostream& operator<< (std::ostream& os, const Test& test)
{
    os << test.name << std::endl;
    return os;
}

std::istream& operator>> (std::istream& is,  Test& test)
{
    is >> test.name ;
    return is;
}

int main()
{
    Test test;
    std::cin >> test;
    std::cout << test;
    ++ test;
    std::cout << test.get_cnt() << std::endl;
    return 0; 
}

/*
class Test {
public:
    // 前缀：返回自增后的引用
    Test& operator++() {
        ++count;
        return *this;
    }
    
    // 后缀：返回自增前的值（副本）
    Test operator++(int) {
        Test temp = *this; // 保存原值
        ++count;           // 执行自增
        return temp;       // 返回自增前的值
    }
};

*/