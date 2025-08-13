#include <iostream>
#include <memory>

// 抽象产品
class Button {
public:
    virtual void render() = 0;
    virtual ~Button() = default;
};

// 具体产品
class WindowsButton : public Button {
public:
    void render() override { std::cout << "Windows Button\n"; }
};

class MacButton : public Button {
public:
    void render() override { std::cout << "Mac Button\n"; }
};

// 抽象创建者
class Dialog {
public:
    virtual std::unique_ptr<Button> createButton() = 0;
    void render() {
        auto button = createButton();
        button->render();
    }
    virtual ~Dialog() = default;
};

// 具体创建者
class WindowsDialog : public Dialog {
public:
    std::unique_ptr<Button> createButton() override {
        return std::make_unique<WindowsButton>();
    }
};

class MacDialog : public Dialog {
public:
    std::unique_ptr<Button> createButton() override {
        return std::make_unique<MacButton>();
    }
};

// 使用示例
int main() {
    std::unique_ptr<Dialog> winDialog = std::make_unique<WindowsDialog>();
    winDialog->render();  // Windows Button

    std::unique_ptr<Dialog> macDialog = std::make_unique<MacDialog>();
    macDialog->render();  // Mac Button
}