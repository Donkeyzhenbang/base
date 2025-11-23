// 策略模式使用的就是面向对象的继承和多态机制，从而实现同一行为在不同场景下具备不同实现。
// 策略模式本质：分离算法，选择实现
/*
策略模式 涉及到三个角色：
环境角色
抽象策略角色
具体策略角色：
*/ 
#include <bits/stdc++.h>

//
//策略模式 采用传统方式实现:class or 模板
//

using std::cout;
using std::endl;

//抽象策略类，提供一个接口
class Hurt {
public:
    virtual void blood() = 0;
};

//具体的策略实现类，具体实现接口， Adc持续普通攻击
class AdcHurt : public Hurt {
public:
    void blood() override {
        cout << "Adc hurt, Blood loss" << endl;
    }
};

//具体的策略实现类，具体实现接口， Apc技能攻击
class ApcHurt : public Hurt {
public:
    void blood() override {
        cout << "Apc Hurt, Blood loss" << endl;
    }
};

//================================================//
//类指针方式
//环境角色类， 游戏角色战士，传入一个策略类指针参数
//采用了shared_ptr,如果采用unique_ptr呢？两者意义是否一样？
class Soldier {
public:
    Soldier(std::shared_ptr<Hurt> hurt) : m_pHurt(hurt) {
    }

    //在不同的策略下，该游戏角色表现出不同的攻击
    void attack() {
        m_pHurt->blood();
    }

private:
    std::shared_ptr<Hurt> m_pHurt;
};

//================================================//
//策略标签+简单工厂模式+策略模式
//定义策略标签
typedef enum {
    Hurt_Type_Adc,
    Hurt_Type_Apc,
    Hurt_Type_Num
} HurtType;

//环境角色类， 游戏角色法师，传入一个策略标签参数。
class Mage {
public:
    Mage(HurtType type) {
        switch (type) {
            case Hurt_Type_Adc:
                m_pHurt = std::make_shared<AdcHurt>();
                break;
            case Hurt_Type_Apc:
                m_pHurt = std::make_shared<ApcHurt>();
                break;
            default:
                break;
        }
    }

    void attack() {
        m_pHurt->blood();
    }

private:
    std::shared_ptr<Hurt> m_pHurt;
};

//================================================//
//模板实现
//环境角色类， 游戏角色弓箭手，实现模板传递策略。
template<typename T>
class Archer {
public:
    void attack() {
        m_hurt.blood();
    }

private:
    T m_hurt;
};

int main() {

    //传入策略类指针
    Soldier soldier(std::make_shared<AdcHurt>());
    soldier.attack();
    //运行结果:
    //Adc hurt, Blood loss

    //传入策略标签,具体策略对象的创建在上下文环境Context中实现
    std::shared_ptr<Mage> pMage = std::make_shared<Mage>(HurtType::Hurt_Type_Adc);
    pMage->attack();
    //运行结果:
    //Adc hurt, Blood loss


    //采用模板实现
    Archer<ApcHurt> *arc = new Archer<ApcHurt>;
    arc->attack();
    delete arc;
    arc = nullptr;
    //运行结果:
    //Apc Hurt, Blood loss

    //基于智能指针的模板
    std::shared_ptr<Archer<AdcHurt>> pArc = std::make_shared<Archer<AdcHurt>>();
    pArc->attack();
    //运行结果:
    //Adc hurt, Blood loss

    return 0;
}
