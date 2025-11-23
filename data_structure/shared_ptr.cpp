#include <iostream>
#include <utility>
using namespace std;

namespace {

template <typename T>
class SharedPtr {
private:
    T* ptr_;
    int* pcount_;
    bool is_combined_;  // 标记是否是make_shared分配的组合内存

    void releaseResource() {
        if (ptr_ != nullptr) {
            (*pcount_)--;
            if (*pcount_ == 0) {
                cout << "Releasing resource. Use count reaches 0" << endl;
                
                // 1. 调用对象析构函数
                ptr_->~T();
                
                // 2. 释放内存（区分普通内存和组合内存）
                if (is_combined_) {
                    // 组合内存：用operator delete释放整个块
                    operator delete(reinterpret_cast<void*>(ptr_));
                } else {
                    // 普通内存：分别释放对象和计数
                    delete ptr_;
                    delete pcount_;
                }
                
                ptr_ = nullptr;
                pcount_ = nullptr;
            }
        } else if (pcount_ != nullptr) {
            delete pcount_;
            pcount_ = nullptr;
        }
    }

    // 友元声明
    template<typename U, typename... Args>
    friend SharedPtr<U> make_shared(Args&&... args);

    // 私有构造函数（仅用于make_shared的组合内存）
    explicit SharedPtr(void* combinedData) : is_combined_(true) {
        ptr_ = static_cast<T*>(combinedData);
        pcount_ = reinterpret_cast<int*>(static_cast<char*>(combinedData) + sizeof(T));
        cout << "SharedPtr(combined) constructed. Use count: " << *pcount_ << endl;
    }

public:
    //! 1. nullptr专用构造函数
    //解决二义性内核 编译器无法确定应该调用：
    //SharedPtr(T* ptr)构造函数（因为 nullptr可转换为 T*）
    //还是拷贝构造函数（如果存在可转换的类型）
    explicit SharedPtr(std::nullptr_t) : ptr_(nullptr), pcount_(new int(0)), is_combined_(false) {
        cout << "SharedPtr(nullptr) constructed. Use count: 0" << endl;
    }

    // 2. 普通原始指针构造函数
    explicit SharedPtr(T* ptr) : ptr_(ptr), is_combined_(false) {
        pcount_ = (ptr != nullptr) ? new int(1) : new int(0);
        cout << "SharedPtr(raw ptr) constructed. Use count: " << *pcount_ << endl;
    }

    // 3. 拷贝构造函数
    SharedPtr(const SharedPtr& s) 
        : ptr_(s.ptr_), pcount_(s.pcount_), is_combined_(s.is_combined_) {
        if (ptr_ != nullptr) {
            (*pcount_)++;
        }
        cout << "SharedPtr copied. Use count: " << *pcount_ << endl;
    }

    // 4. 赋值运算符
    SharedPtr<T>& operator=(const SharedPtr& s) {
        if (this != &s) {
            releaseResource();
            ptr_ = s.ptr_;
            pcount_ = s.pcount_;
            is_combined_ = s.is_combined_;
            if (ptr_ != nullptr) {
                (*pcount_)++;
            }
            cout << "SharedPtr assigned. Use count: " << *pcount_ << endl;
        }
        return *this;
    }

    // 5. 解引用运算符
    T& operator*() {
        if (ptr_ == nullptr) {
            throw runtime_error("Dereference null SharedPtr");
        }
        return *ptr_;
    }

    // 6. 成员访问运算符
    T* operator->() {
        if (ptr_ == nullptr) {
            throw runtime_error("Access member via null SharedPtr");
        }
        return ptr_;
    }

    // 7. 引用计数
    int use_count() const {
        return *pcount_;
    }

    // 8. 析构函数
    ~SharedPtr() {
        cout << "SharedPtr destructor called. Current use count: " << *pcount_ << endl;
        releaseResource();
    }
};

// make_shared实现
template<typename T, typename... Args>
SharedPtr<T> make_shared(Args&&... args) {
    size_t totalSize = sizeof(T) + sizeof(int);
    void* combinedData = operator new(totalSize);  //! 分配原始内存 注意这里使用operator new分配对象+计数器内存 一并分配后续一并删除

    try {
        //! 在原始内存上构造对象（placement new）
        T* objPtr = new(combinedData) T(forward<Args>(args)...);
        
        // 初始化引用计数
        int* countPtr = reinterpret_cast<int*>(static_cast<char*>(combinedData) + sizeof(T));
        *countPtr = 1;
        
        return SharedPtr<T>(combinedData);
    } catch (...) {
        operator delete(combinedData);  // 异常时释放内存
        throw;
    }
}

}  // namespace

class TestClass {
private:
    int value;
public:
    TestClass(int val) : value(val) {
        cout << "TestClass(" << value << ") constructed" << endl;
    }
    
    ~TestClass() {
        cout << "TestClass(" << value << ") destroyed" << endl;
    }
    
    void print() const {
        cout << "TestClass.value = " << value << endl;
    }
    
    int getValue() const { return value; }
    void setValue(int v) { value = v; }
};

void testSharedPtr() {
    cout << "\n===== Test Case 1: Basic Construction (new) =====" << endl;
    {
        SharedPtr<TestClass> p1(new TestClass(10));
        p1->print();
        cout << "Use count: " << p1.use_count() << endl;
    }

    cout << "\n===== Test Case 2: make_shared Construction =====" << endl;
    {
        SharedPtr<TestClass> p2 = make_shared<TestClass>(20);
        p2->print();
        cout << "Use count (make_shared): " << p2.use_count() << endl;

        SharedPtr<TestClass> p3 = p2;
        cout << "p2 use count: " << p2.use_count() << endl;
        cout << "p3 use count: " << p3.use_count() << endl;
    }

    cout << "\n===== Test Case 3: Assignment =====" << endl;
    {
        SharedPtr<TestClass> p1 = make_shared<TestClass>(30);
        SharedPtr<TestClass> p2(new TestClass(40));
        
        cout << "Before assignment:" << endl;
        cout << "p1 use count: " << p1.use_count() << endl;
        cout << "p2 use count: " << p2.use_count() << endl;
        
        p2 = p1;
        
        cout << "After assignment:" << endl;
        cout << "p1 use count: " << p1.use_count() << endl;
        cout << "p2 use count: " << p2.use_count() << endl;
    }

    cout << "\n===== Test Case 4: Multiple Copies =====" << endl;
    {
        auto p1 = make_shared<TestClass>(50);
        auto p2 = p1;
        auto p3 = p2;

        cout << "p1 use count: " << p1.use_count() << endl;
        cout << "p2 use count: " << p2.use_count() << endl;
        cout << "p3 use count: " << p3.use_count() << endl;

        p1->setValue(55);
        p3->print();
    }

    cout << "\n===== Test Case 5: Null Pointer =====" << endl;
    {
        SharedPtr<TestClass> p1(nullptr);
        cout << "Null pointer use count: " << p1.use_count() << endl;
    }
}

int main() {
    testSharedPtr();
    return 0;
}
