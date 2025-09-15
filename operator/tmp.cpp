#include <iostream>
using namespace std;

void fun(char* p) {
    cout << "char* p" << endl;
}

void fun(int* p) {
    cout << "int* p" << endl;
}

void fun(int p) {
    cout << "int p" << endl;
}

int main() {
    fun(static_cast<char*>(0));    // 明确调用 char* 版本
    fun(static_cast<int*>(nullptr)); // 明确调用 int* 版本
    fun(0);                       // 明确调用 int 版本
    fun((char*)nullptr);

    return 0;
}