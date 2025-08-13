#include <iostream>
#include <memory>
#include <string>
#include <list>

class Student{
private:
    double score;
    std::string name;
public:
    double get_score() 
    {
        return this->score;
    }
    std::string& get_name()
    {
        return this->name;
    }
    void set(const std::string& n, double s)
    {
        this->name = n;
        this->score = s;
    }
    void display()
    {
        std::cout << "Student " << this->name << " score is " << this->score << std::endl; 
    }
};

/*使用{}可以聚合初始化，但是变量必须要求是public
    聚合类型在 C++ 标准中有严格的定义，以下是聚合类型的条件：
    没有用户定义的构造函数。
    所有非静态数据成员都是 public。
    没有虚函数。
    没有 private 或 protected 的非静态数据成员。
*/
// struct Message
// {
//     int x;
//     int y;
//     void speak() {
//         std::cout << "Move " << x << ", " << y << '\n';
//     }
//     void happy() const { std::cout << "MoveMsg is happy\n"; }
// };
class Message
{
private:
public:
    int x;
    int y;
    // Message(int x, int y) : x(x), y(y) {}
    void speak() {
        std::cout << "Move " << x << ", " << y << '\n';
    }
    void happy() const { std::cout << "MoveMsg is happy\n"; }
};

struct MsgBase
{
    virtual void speak() = 0;
    virtual void happy() = 0;
    // virtual std::shared_ptr<MsgBase> clone const = 0;
};

template <class Message>
struct MsgTest : MsgBase{
    Message msg;
    template <class ...Ts>
    MsgTest(Ts &&...ts) : msg{std::forward<Ts>(ts)...} {
    }

    void speak() {
        msg.speak();
    }
    void happy() {
        msg.happy();
    }

};

/*
类型转换 1：std::make_shared<MsgTest<Message>>(5, 10) 会返回一个 std::shared_ptr<MsgTest<Message>>。
类型转换 2：std::shared_ptr<MsgTest<Message>> 被隐式转换为 std::shared_ptr<MsgBase>，这是 makeMsg 函数的最终返回类型。
1. std::make_shared<MsgTest<Message>>(std::forward<Ts>(ts)...)
创建 MsgTest<Message> 对象：首先，通过 std::make_shared 创建一个 MsgTest<Message> 对象。
完美转发 Ts&&... 参数：std::forward<Ts>(ts)... 会将所有传入的 ts 参数按照完美转发的规则，转发到 MsgTest<Message> 的构造函数。
完美转发会确保参数的左右值属性保持不变（如左值传左值，右值传右值），以避免不必要的拷贝。
类型转换 1：std::make_shared<MsgTest<Message>> 的返回类型是 std::shared_ptr<MsgTest<Message>>，这是一个具体类型的智能指针。
2. 将 std::shared_ptr<MsgTest<Message>> 转换为 std::shared_ptr<MsgBase>
由于 MsgTest<Message> 继承自 MsgBase，因此可以将 std::shared_ptr<MsgTest<Message>> 隐式转换为 std::shared_ptr<MsgBase>。
类型转换 2：std::shared_ptr<MsgTest<Message>> 被隐式转换为 std::shared_ptr<MsgBase>，这是因为 std::shared_ptr 支持向上类型转换（继承树中的上层基类指针）。
这一转换实现了多态的效果，允许我们将不同的 MsgTest<Message> 对象作为 MsgBase 指针来统一管理。
*/
template <class Message, class ...Ts>
std::shared_ptr<MsgBase> makeMsg(Ts &&...ts) {
    return std::make_shared<MsgTest<Message>>(std::forward<Ts>(ts)...);
}


int main()
{
    std::unique_ptr<Student> stu_ptr = std::make_unique<Student>();
    stu_ptr->set("Jack", 90);
    stu_ptr->display();
    std::shared_ptr<MsgBase> msg_ptr = makeMsg<Message>(5, 10);
    msg_ptr->speak();
    // Message msg{5, 10};
    // msg.speak();
    return 0;
}