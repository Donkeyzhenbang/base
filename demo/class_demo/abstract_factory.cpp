#include <iostream>
#include <memory>

// 抽象产品族
class Checkbox {
public:
    virtual void check() = 0;
    virtual ~Checkbox() = default;
};

class TextField {
public:
    virtual void input() = 0;
    virtual ~TextField() = default;
};

// 具体产品族 - Windows
class WindowsCheckbox : public Checkbox {
public:
    void check() override { std::cout << "Windows Checkbox\n"; }
};

class WindowsTextField : public TextField {
public:
    void input() override { std::cout << "Windows TextField\n"; }
};

// 具体产品族 - Mac
class MacCheckbox : public Checkbox {
public:
    void check() override { std::cout << "Mac Checkbox\n"; }
};

class MacTextField : public TextField {
public:
    void input() override { std::cout << "Mac TextField\n"; }
};

// 抽象工厂
class GUIFactory {
public:
    virtual std::unique_ptr<Checkbox> createCheckbox() = 0;
    virtual std::unique_ptr<TextField> createTextField() = 0;
    virtual ~GUIFactory() = default;
};

// 具体工厂
class WindowsFactory : public GUIFactory {
public:
    std::unique_ptr<Checkbox> createCheckbox() override {
        return std::make_unique<WindowsCheckbox>();
    }
    
    std::unique_ptr<TextField> createTextField() override {
        return std::make_unique<WindowsTextField>();
    }
};

class MacFactory : public GUIFactory {
public:
    std::unique_ptr<Checkbox> createCheckbox() override {
        return std::make_unique<MacCheckbox>();
    }
    
    std::unique_ptr<TextField> createTextField() override {
        return std::make_unique<MacTextField>();
    }
};

// 使用示例
int main() {
    std::unique_ptr<GUIFactory> factory = std::make_unique<WindowsFactory>();
    auto checkbox = factory->createCheckbox();
    auto textfield = factory->createTextField();
    
    checkbox->check();    // Windows Checkbox
    textfield->input();   // Windows TextField

    factory = std::make_unique<MacFactory>();
    checkbox = factory->createCheckbox();
    textfield = factory->createTextField();
    
    checkbox->check();    // Mac Checkbox
    textfield->input();   // Mac TextField
}