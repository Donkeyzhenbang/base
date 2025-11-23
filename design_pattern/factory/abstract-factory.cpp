/**
工厂方法解决了简单工厂的问题（无法对修改关闭），但是它也有自己的局限：
试想一下，宝马工厂里面只是售卖成品汽车吗？
应该不是吧，作为一家成熟的工厂，除了汽车之外，还有一系列配套的零件、产品：比如说：轮胎、车灯、真皮豪华座椅等等。也就是说，跟汽车相关联的有一整个产品簇。

但是我们的宝马工厂BmwFac里面只有一个createCar方法，如果想要添加产品的话，就需要增加新的类。但是这些产品其实都应该在一个BmwFac工厂里面。这才是现实的逻辑，另外，工厂类太多，会不好维护。

于是乎，抽象工厂 应运而生。

抽象工厂的思想是：
把有关联关系的，属于一个产品簇的所有产品创建的接口函数，放在一个抽象工厂里面AbstractFactory，派生类（具体产品的工厂）应该负责创建该产品簇里面所有的产品。
*/

#include <iostream>
#include <memory>
#include <string>

// 系列产品1：汽车
class Car
{
public:
	Car(std::string name) : name_(name) {}
	virtual void show() = 0;
protected:
	std::string name_;
};

class Bmw : public Car
{
public:
	Bmw(std::string name) : Car(name) {}
	void show()
	{
		std::cout << "获取了一辆宝马汽车：" << name_ << std::endl;
	}
};
class Audi :public Car
{
public:
	Audi(std::string name) : Car(name) {}
	void show()
	{
		std::cout << "获取了一辆奥迪汽车：" << name_ << std::endl;
	}
};

// 系列产品2：车灯
class Light
{
public:
	virtual void show() = 0;
};

class BmwLight : public Light
{
public:
	void show() { std::cout << "BMW light!" << std::endl; }
};

class AudiLight : public Light
{
public:
	void show() { std::cout << "Audi light!" << std::endl; }
};

class AbstractFactory {
public:
    std::unique_ptr<Car> virtual CreateCar(std::string name) = 0;
    std::unique_ptr<Light> virtual CreateLight() = 0;
};

class BMWFactory : public AbstractFactory {
public:
    std::unique_ptr<Car> CreateCar(std::string name) override {
        return std::make_unique<Bmw>(name);
    }

    std::unique_ptr<Light> CreateLight() override {
        return std::make_unique<BmwLight>();
    }

};

class AUDIFactory : public AbstractFactory {
public:
    std::unique_ptr<Car> CreateCar(std::string name) override {
        return std::make_unique<Audi>(name);
    }

    std::unique_ptr<Light> CreateLight() override {
        return std::make_unique<AudiLight>();
    }
};

int main()
{
	std::unique_ptr<AbstractFactory> bmw_fac = std::make_unique<BMWFactory>();
	std::unique_ptr<AbstractFactory> audi_fac = std::make_unique<AUDIFactory>();
	std::unique_ptr<Car> bmw_car = bmw_fac->CreateCar("X6");
	std::unique_ptr<Car> audi_car = audi_fac->CreateCar("A8");
	std::unique_ptr<Light> bmw_light = bmw_fac->CreateLight();
	std::unique_ptr<Light> audi_light = audi_fac->CreateLight();
	bmw_car->show();
	bmw_light->show();
	audi_car->show();
	audi_light->show();
    return 0;
}