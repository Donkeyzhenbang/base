// 抽象工厂 ： 把有关联关系的，属于一个产品簇的所有产品创建的接口函数，放在一个抽象工厂里面AbstractFactory，派生类（具体产品的工厂）应该负责创建该产品簇里面所有的产品。

#include <iostream>
#include <string>
#include <memory>

class Car {
protected:
    std::string name_;
public:
    Car(std::string name) : name_(name) {}
    virtual void show() = 0;
};

class Bmw : public Car {
public: 
    Bmw(std::string name) : Car(name) {}
    void show() override {
        std::cout << "Bmw Car name is " << name_ << std::endl;
    }

};

class Audi : public Car {
public: 
    Audi(std::string name) : Car(name) {}
    void show() override {
        std::cout << "Audi Car name is " << name_ << std::endl;
    }
};

enum CarType {
    BMW, AUDI
};

class SimpleFactory {
public: 
    Car* CreateCar(CarType type) {
        switch(type){
            case BMW :
                return new Bmw("x6");
            case AUDI :
                return new Audi("A8");
            default:
                std::cerr << "传入参数错误 ！" << std::endl;
        }
        return nullptr;
    }

    std::unique_ptr<Car> CreateSmartCar(CarType type) {
        switch(type){
            case BMW :
                return std::make_unique<Bmw>("X6");
            case AUDI :
                return std::make_unique<Audi>("A8");
            default:
                std::cerr << "传入参数错误 ！" << std::endl;
        }
        return nullptr;
    }
};

int main()
{
    std::unique_ptr<SimpleFactory> factory = std::make_unique<SimpleFactory>();
    // 返回原始指针 无法转化成智能指针
    //我们无法直接将原始指针赋值给 unique_ptr，因为 unique_ptr 的构造函数是显式的。
    //但是，我们可以通过使用 std::unique_ptr 的构造函数来接管原始指针，或者使用 std::make_unique 来创建智能指针。
    auto c1 = factory->CreateCar(BMW);
    auto c2 = factory->CreateCar(AUDI);

    c1->show();
    c2->show();


    std::unique_ptr<Car> s1 = factory->CreateSmartCar(BMW);
    std::unique_ptr<Car> s2 = factory->CreateSmartCar(AUDI);
    // std::unique_ptr<Car> c1 = std::unique_ptr<Car>(factory->CreateCar(BMW));
    s1->show();
    s2->show();
    return 0;
}