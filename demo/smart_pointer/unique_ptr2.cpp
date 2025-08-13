#include <iostream>
#include <memory>
#include <string>
#include <vector>

class Student {
private:
    std::string name;
    double score;

public:
    Student() = default;

    Student(const std::string& n, double s) : name(n), score(s) {
        std::cout << "Constructed: " << name << "\n";
    }
    // 拷贝构造函数 拷贝构造：复制资源，两个对象拥有自己的副本，可能导致性能问题。
    Student(const Student& other) : name(other.name), score(other.score) {
        std::cout << "Copied: " << name << "\n";
    }

    // 移动构造函数 移动构造：转移资源的所有权，通常更加高效，源对象在移动后可能处于未定义状态。
    Student(Student&& other) noexcept 
        : name(std::move(other.name)), score(other.score) {
        std::cout << "Moved: " << name << "\n";
        other.score = 0; // 清空源对象的状态
        other.name = "Unknown";
    }

    void set(const std::string& n, double s) {
        this->name = n;
        this->score = s;
    }

    double get_score() const { return score; }

    const std::string& get_name() const { return name; }

    void display() const {
        std::cout << "name: " << name << "\t score: " << score << std::endl;
    }

    ~Student() {
        std::cout << "Destroyed: " << name << "\n";
    }
};

int compare(const std::unique_ptr<Student>& a, const std::unique_ptr<Student>& b) {
    if (a->get_score() < b->get_score()) return 1;
    if (a->get_score() > b->get_score()) return -1;
    return 0;
}

int main() {
    Student student1("Ros", 95);
    Student student2 = std::move(student1); // 这里触发了移动构造

    student2.display(); // 输出 "Rose" 和 "95"

    std::vector<std::unique_ptr<Student>> students;
    std::string stu_name[] = { "Rose", "Mike", "Eve", "Micheal", "Jack" };
    double stu_score[] = { 95, 84, 88, 64, 100 };

    // 动态创建学生对象并存储在智能指针中
    //移动构造的触发
    //std::unique_ptr 的特性：std::unique_ptr 本质上是一个智能指针，负责管理动态分配的内存。
    //当你将一个 std::unique_ptr 移动到另一个 std::unique_ptr 时，只有源智能指针的所有权会转移到目标智能指针，源智能指针将变为 nullptr，而不是进行对象的复制。

    //std::move 的作用：std::move(student) 将 student 转换为右值引用，表示它的资源可以被转移。
    //在 students.push_back(std::move(student)) 时，向量内部会处理这个右值，直接将 student 的管理权转移到向量中，而不是通过复制构造函数创建新对象。
    for (int i = 0; i < 5; i++) {
        auto student = std::make_unique<Student>();
        student->set(stu_name[i], stu_score[i]);
        student->display();
        students.push_back(std::move(student));
    }

    // 使用智能指针进行比较
    auto min = students[0].get();
    auto max = students[0].get();

    for (int i = 1; i < students.size(); i++) {
        std::cout << "经过一轮循环 \n";
        if (compare(students[i], std::make_unique<Student>(*min)) == 1) {
            min = students[i].get();
        }
        if (compare(students[i], std::make_unique<Student>(*max)) == -1) {
            max = students[i].get();
        }
    }

    std::cout << "The worst student: " << min->get_name() << std::endl;
    std::cout << "The best student: " << max->get_name() << std::endl;

    return 0;
}
