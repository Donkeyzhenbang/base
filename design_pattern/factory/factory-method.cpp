// 工厂方法 避免简单工厂每新增一个类就要新增类的内容的问题
// 定义一个Factory基类 基类定义纯虚函数 之后在派生类中（具体产品的工厂）负责创建对应的产品
// 可以做到不同的产品在不同的工厂里面创建，能够对现有工厂以及产品的修改关闭。

#include <iostream>
#include <string>
#include <memory>

enum CarType{
    BMW, AUDI
};

class Car {
public:
    Car(std::string name) : name_(name) {}
    virtual void show() = 0;
protected:
    std::string name_;
};

class Bmw : public Car {
public:
    Bmw(std::string name) : Car(name) {}
    void show() override {
        std::cout << "Bmw : " << name_ << std::endl;
    }
};

class Audi : public Car {
public:
    Audi(std::string name) : Car(name) {}
    void show() override {
        std::cout << "Audi : " << name_ << std::endl;
    }
};

class Factory {
public:
    virtual std::unique_ptr<Car> CreateCar(std::string name) = 0;
};

class BmwFac : public Factory {
public:
    std::unique_ptr<Car> CreateCar(std::string name) override {
        return std::make_unique<Bmw>(name);
    }
};

class AudiFac : public Factory {
public:
    std::unique_ptr<Car> CreateCar(std::string name) override {
        return std::make_unique<Audi>(name);
    }
};

int main()
{
    std::unique_ptr<Factory> bmw_fac = std::make_unique<BmwFac>();
    std::unique_ptr<Factory> audi_fac = std::make_unique<AudiFac>();
    std::unique_ptr<Car> p1 = bmw_fac->CreateCar("X6");
    std::unique_ptr<Car> p2 = audi_fac->CreateCar("A8");
    p1->show();
    p2->show();
}

