#include <memory>
#include <vector>
#include "virtual_template.h"

/*
浅拷贝与深拷贝
浅拷贝：

浅拷贝仅复制对象中的基本数据成员值。如果数据成员中包含指针或引用，它们只是复制了地址，不会复制指针指向的实际数据。
浅拷贝会导致两个对象共享相同的底层数据。如果其中一个对象修改了数据，另一个对象也会受到影响。
默认的拷贝构造函数（由编译器自动生成的）通常就是浅拷贝。
深拷贝：

深拷贝不仅复制对象中的数据成员，还会为每一个指针或引用成员分配新的内存，并复制指针指向的内容。
深拷贝使得两个对象拥有独立的数据，修改其中一个不会影响另一个。
实现深拷贝需要手动编写拷贝构造函数，确保每个指针成员指向的数据都被正确地复制。
*/

//MsgBase 是一个抽象基类，用于定义一个通用接口，使不同消息类型都可以通过相同的接口调用。
struct MsgBase {
    virtual void speak() = 0;
    virtual void happy() = 0;
    virtual std::shared_ptr<MsgBase> clone() const = 0;
    virtual ~MsgBase() = default;
};

/*  
    MsgImpl<Msg>：适配器类模板，用于将任意具体消息类型（如 MoveMsg）封装到 MsgBase 接口中。通过模板参数 Msg 泛型化，能够适应任意消息类型的构造。
    Msg 并不是在代码中定义的类，而是一个占位符，用来表示具体的消息类型，比如 MoveMsg、JumpMsg 等。
    这些类可以具有不同的构造参数以及不同的 speak() 和 happy() 实现。
    通过模板参数 <Msg> 实现了对任意消息类型的适配。
*/
template <class Msg>
struct MsgImpl : MsgBase {
    Msg msg; //模板参数 Msg 代表具体的消息类型，例如 MoveMsg、JumpMsg
    /*
    该构造函数使用了模板参数包 Ts...，可以接收任意数量和类型的参数。
    std::forward 用于完美转发参数，以保持参数的原始类型（左值或右值），构造出消息类型 Msg 的对象 msg。
    作用：通过这种设计，MsgImpl 可以适应任意构造参数的 Msg 对象，实现通用性。
    */
    template <class ...Ts>
    MsgImpl(Ts &&...ts) : msg{std::forward<Ts>(ts)...} {
    }

    void speak() override {
        msg.speak();
    }

    void happy() override { // 添加 happy 函数的实现
        msg.happy();
    }

    /*
    1.创建新实例：std::make_shared<MsgImpl<Msg>>(msg) 会创建一个新的 MsgImpl<Msg> 对象，并将 msg 作为参数传递给 MsgImpl<Msg> 的构造函数。

    2.复制构造：MsgImpl<Msg> 的构造函数接受一个 Msg 类型的参数，而 msg 是当前对象的成员变量（类型为 Msg）。
    当 msg 被传递给构造函数时，默认的行为是使用复制构造函数来创建新的 Msg 对象，这样就形成了一个新的独立的 Msg 副本。

    3.独立的内存分配：新创建的 MsgImpl<Msg> 对象和原来的 MsgImpl<Msg> 对象位于不同的内存地址，彼此独立。
    即使 msg 是一个包含指针或其他复杂结构的数据成员，新的 MsgImpl<Msg> 对象将拥有自己的 msg 副本，而不是与原对象共享同一个 msg。 
    */
    std::shared_ptr<MsgBase> clone() const override {
        return std::make_shared<MsgImpl<Msg>>(msg);
    }
};

//MsgImpl 是一个模板类，需要一个模板参数 Msg 来指定具体类型。
//如果 Msg 是 MoveMsg，那么 MsgImpl<Msg> 就相当于 MsgImpl<MoveMsg>，表示 MsgImpl 专门化成包含 MoveMsg 类型的实现。
template <class Msg, class ...Ts>
std::shared_ptr<MsgBase> makeMsg(Ts &&...ts) {
    return std::make_shared<MsgImpl<Msg>>(std::forward<Ts>(ts)...);
}

// struct MsgFactoryBase {
//     virtual std::shared_ptr<MsgBase> makeMsg() = 0;
// };

// template <class Msg>
// struct MsgFactoryImpl {
// };

std::vector<std::shared_ptr<MsgBase>> msgs;

int main() {
    msgs.push_back(makeMsg<MoveMsg>(5, 10));
    msgs.push_back(makeMsg<JumpMsg>(20));
    msgs.push_back(makeMsg<SleepMsg>(8));
    msgs.push_back(makeMsg<ExitMsg>());

    for (auto &msg : msgs) {
        msg->speak();
    }

    return 0;
}