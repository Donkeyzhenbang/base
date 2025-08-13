#pragma once

#include <iostream>

//虽然MoveMsg 是一个 struct，且未显式定义构造函数，但编译器会自动生成一个默认的构造函数和一个成员初始化构造函数，只要 MoveMsg 中的成员变量都是可以直接初始化的类型（例如 int）。
struct MoveMsg {
    int x;
    int y;

    void speak() {
        std::cout << "Move " << x << ", " << y << '\n';
    }
    void happy() const { std::cout << "MoveMsg is happy\n"; }
};

struct JumpMsg {
    int height;

    void speak() {
        std::cout << "Jump " << height << '\n';
    }
    void happy() const { std::cout << "JumpMsg is happy\n"; }
};

struct SleepMsg {
    int time;

    void speak() {
        std::cout << "Sleep " << time << '\n';
    }
    void happy() const { std::cout << "SleepMsg is happy\n"; }
};

struct ExitMsg {
    void speak() {
        std::cout << "Exit" << '\n';
    }
    void happy() const { std::cout << "ExitMsg is happy\n"; }
};