#include <iostream>
#include <string>
#include <vector>
#include <memory>

/* //!智能指针与RAII
RAII是一种编程技术，用于管理资源（如内存、文件句柄、网络连接等），其核心思想是将资源的生命周期与对象的生命周期绑定。
当对象被创建时，它获取必要的资源；当对象被销毁时，它释放这些资源。这样可以保证即使发生异常，资源也能被正确地释放。

智能指针是一种特殊的指针，它封装了原始指针，并在对象生命周期结束时自动释放其指向的资源。C++标准库提供了几种智能指针，如std::unique_ptr、std::shared_ptr和std::weak_ptr。
智能指针利用了RAII的概念，它们在构造时获取资源（通常是通过new操作符分配内存），并在析构时释放资源（通过delete操作符）。

智能指针是实现RAII的一种方式。通过使用智能指针，你可以确保动态分配的内存在不再需要时被自动释放，从而避免内存泄漏。
*/

/* //!const修饰符
    const 修饰成员函数仅用于类中的成员函数时，其作用是保证该成员函数不会修改所属对象的状态
    void display() const {
    std::cout << "Student: " << name << ", Score: " << score << std::endl;
}
    const 修饰返回值类型和函数参数：const std::string&，表示返回的引用是不可修改的。
    调用者不能修改通过这个函数返回的 name，只能读取它。
    如果去掉 const，调用者将有权限修改返回的字符串引用。
    const std::string& get_name() const { return name; }
*/

/* //!default
    显式默认构造函数
    Student() = default;
    构造函数：如 Student() = default;，告诉编译器自动生成默认的无参构造函数。
    析构函数：~Student() = default;，告诉编译器自动生成默认的析构函数。
    拷贝构造函数和拷贝赋值运算符：如果没有特别的要求，使用 = default 可以让编译器生成默认的拷贝构造和赋值运算符。
    当你需要让类有特定行为（如自定义构造、析构等），但仍然希望保留编译器生成的某些默认行为时，可以用 = default 显式要求编译器生成这些函数。
*/


/* //!智能指针 reset make_unique
    std::unique_ptr 提供了一个 reset() 方法，用于重置智能指针的管理对象或将其指向新的对象。
    void reset(Student* ptr = nullptr);
    作用：reset 用于替换当前管理的指针对象。如果智能指针已经持有一个对象，reset() 会释放当前管理的对象，然后将其指向新对象（或为空）。
    参数：它接受一个原始指针 ptr，该指针将成为新的管理对象。如果不传参数，则会将当前指针对象重置为 nullptr，释放原来的资源。

    在 std::make_unique<Student>() 中，<> 表示这是一个模板函数的调用。具体来说，std::make_unique 是一个模板，Student 是其模板参数，表示要创建的对象类型。
    在 std::make_unique<Student>() 的末尾有一个空的圆括号 ()，这表示调用 make_unique 函数
    std::make——unique原型代码如下：
    template<typename _Tp, typename... _Args>
    inline typename _MakeUniq<_Tp>::__single_object
    make_unique(_Args&&... __args)
    { return unique_ptr<_Tp>(new _Tp(std::forward<_Args>(__args)...)); }
    
    1.代码分析：
    模板参数：
    typename _Tp：表示要创建的对象类型（比如 Student）。
    typename... _Args：表示任意数量的参数，允许传递构造对象所需的任意数量和类型的参数。

    返回类型：
    typename _MakeUniq<_Tp>::__single_object：这里使用了一个类型别名，通常是 std::unique_ptr<_Tp>。这个返回类型表示这个函数返回一个指向 _Tp 类型的智能指针。

    函数体：
    new _Tp(std::forward<_Args>(__args)...)：这里调用了 _Tp 的构造函数，传递了使用 std::forward 转发的参数。std::forward 用于完美转发参数，保持参数的值类别（左值或右值）。
    return unique_ptr<_Tp>(...)：创建并返回一个 std::unique_ptr，它管理新创建的对象的生命周期。

    2. 与 std::make_unique<Student>() 的关系
    当你调用 std::make_unique<Student>() 时，发生的事情如下：
    Student 被推断为 _Tp，即要创建的对象类型。
    空的 () 表示没有构造参数，因此 _Args 为零参数。
    这个调用实际上转发了空的参数给 make_unique 函数，即 __args 是一个空包（没有参数）。

    3. 小括号和尖括号的区别
    尖括号 <>：用于指明模板参数类型。在 std::make_unique<Student>() 中，尖括号用于指定模板类型 Student，这是模板函数的类型参数。

    小括号 ()：用于调用函数或构造对象。在 std::make_unique<Student>() 中，小括号表示调用 make_unique 函数。如果有参数，则小括号中会包含这些参数；如果没有参数，如 std::make_unique<Student>()，则小括号为空。

    4. 参数数量的不同
    在 make_unique 的实现中，使用了可变参数模板 (typename... _Args) 来支持任意数量的构造参数。
    在调用 std::make_unique<Student>() 时，如果没有提供参数，_Args 就是一个空包，允许你创建一个 Student 对象的默认实例。

    总结
    std::make_unique<Student>() 通过模板函数 make_unique 实现了创建智能指针的功能。
    尖括号 <> 用于指定模板类型，而小括号 () 用于调用函数。
    make_unique 的实现灵活地支持任意数量的构造参数，允许你根据需要创建不同的对象。通过这种方式，std::make_unique 不仅提高了代码的安全性和可读性，也减少了手动内存管理的负担。
*/

/* //!std::move vector.push_back()
    使用 std::move显式地将一个对象转换为右值引用，从而启用移动语义。它不会真的移动对象，而是指示编译器可以安全地窃取资源。
    将 std::unique_ptr<Student> 移入 vector 中，因为 unique_ptr 不允许拷贝。                                                                                   7
    在 C++ 中，std::unique_ptr 是不可拷贝的。也就是说，不能对 unique_ptr 执行拷贝操作，例如：

    std::unique_ptr<Student> p1(new Student());
    std::unique_ptr<Student> p2 = p1;  // 错误：不能拷贝 unique_ptr
    如果你想要将 p1 的所有权转移给 p2，必须使用移动语义，即使用 std::move()，将 p1 变成右值引用：

    std::unique_ptr<Student> p2 = std::move(p1);  // 合法：p1 的所有权转移到 p2
    在这之后，p1 会变为空 (nullptr)，而 p2 将接管 p1 原本指向的资源。

    students.push_back(std::move(student));
    将 unique_ptr 对象放入 std::vector 中，原因如下：
    1.unique_ptr 不可拷贝：由于 unique_ptr 是独占的，不能进行拷贝操作。如果直接尝试 students.push_back(student)，编译器会报错，因为 push_back 要求对 student 进行拷贝
    2.转移所有权：使用 std::move(student)，你告诉编译器，student 的资源可以被转移到 students 容器中，student 之后将不再拥有资源（即变为空）。
*/

class Student{
private:
    std::string name;
    double score;
public:
    Student() = default;
    void set(const std::string& n, double s)
    {
        this->name = n;
        this->score = s;
    }
    double get_score() const {return score;}
    const std::string& get_name() const {return name;}
    void display() const {
        std::cout << "name : " << name << "\t score " << score << std::endl;
    }
    ~Student(){

    }
};

int compare(const std::unique_ptr<Student>& a, const std::unique_ptr<Student>& b)
{
    if(a->get_score() < b->get_score()) return 1;
    if(a->get_score() > b->get_score()) return -1;
    return -1;
}


int main()
{
    std::unique_ptr<Student> ptr1(new Student());
    std::unique_ptr<Student> ptr2 = std::make_unique<Student>();
    //max(new Student())和max(new Student)效果一样 都使用默认构造函数
    //!智能指针写法 两种后者更好一些 
    // std::unique_ptr<Student> max(new Student());
    // std::unique_ptr<Student> min(new Student);

    // std::unique_ptr<Student> min = std::make_unique<Student>();
    // std::unique_ptr<Student> max = std::make_unique<Student>();
    
    //!这样是开辟在栈上；注意野指针 由于最后还是会指向栈上的指针，这里就不必要new开辟内存
    // Student *min = nullptr;
    // Student *max = nullptr; 

    //! 下面这种写法是最傻逼的，一个变量一行!//
    // Student *min,*max = new Student();
    //! 这样语法正确，但是此场景存在内存泄漏，因为max min后面改变指针指向，指向了栈上的元素
    // Student *min =  new Student();
    // Student *max =  new Student();

    
    std::vector<std::unique_ptr<Student>> students;
    std::string stu_name[] =  { "Rose","Mike","Eve","Micheal","Jack" };
    double stu_score[] = { 95,84,88,64,100 };

    // 动态创建学生对象并存储在智能指针中
    for(int i = 0; i < 5; i ++){
        //使用 std::make_unique<Student>() 创建动态分配的学生对象，确保它们都在堆上分配。
        auto student = std::make_unique<Student>();
        student->set(stu_name[i], stu_score[i]);
        student->display();
        students.push_back(std::move(student));
    }

    std::unique_ptr<Student> *min = &students[0];
    std::unique_ptr<Student> *max = &students[0];

    for(int i = 0; i < 5; i ++){
        if(compare(*min, students[i]) == 1)
            min = &students[i];   // min指向新的智能指针
        if(compare(*max, students[i]) == -1)
            max = &students[i];   // max指向新的智能指针
    }

    //!min 和 max 指向 std::unique_ptr<Student> 的指针 解引用一层才是指向Student的指针 才可以使用->
    std::cout << "The worst student : " << (*min)->get_name() << std::endl;
	std::cout << "The best student : " << (*max)->get_name() << std::endl;
    return 0;
}

