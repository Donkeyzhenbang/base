#include <bits/stdc++.h>

//
//策略模式 采用函数指针 or std::function实现
//


void adcHurt() {
    std::cout << "Adc Hurt" << std::endl;
}

void apcHurt() {
    std::cout << "Apc Hurt" << std::endl;
}

//环境角色类， 使用传统的函数指针
class Soldier {
public:
    typedef void (*Function)();

    Soldier(Function fun) : m_fun(fun) {
    }

    void attack() {
        m_fun();
    }

private:
    Function m_fun;
};

//环境角色类， 使用std::function<>
class Mage {
public:
    typedef std::function<void()> Function;

    Mage(Function fun) : m_fun(fun) {
    }

    void attack() {
        m_fun();
    }

private:
    Function m_fun;
};

int main() {
    //函数指针实现
    Soldier *soldier = new Soldier(apcHurt);
    soldier->attack();
    delete soldier;
    soldier = nullptr;
    //运行结果:
    //Apc Hurt

    //std::function实现
    std::shared_ptr<Mage> pMage = std::make_shared<Mage>(adcHurt);
    pMage->attack();
    //运行结果:
    //Adc Hurt


    return 0;
}
