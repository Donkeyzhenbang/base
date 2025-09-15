#include <iostream>
#include <initializer_list>
#include <stdexcept>

/**
 * @brief vector简单实现 需要实现api : vector构造/析构 拷贝移动构造/移动赋值 resize 
 * push_back pop_back insert erase clear []运算符 size capcity empty print
 * 
 */
namespace {
template <typename T>
class Vector{
private:
    T* data;            //存储元素的数组
    size_t size;        //当前存储的容量
    size_t capacity;    //当前分配的元素数量

    void resize(size_t newCapacity) {
        //当我们用 new T[newCapacity] 分配内存时，返回的是一个指向数组首元素的指针（类型为 T*）。
        // 虽然 newData 是指针类型，但 C++ 允许通过下标运算符 [] 来访问指针指向的数组元素，这是一种语法糖
        T *newData = new T[newCapacity];
        for(size_t i = 0; i < size; i ++){
            newData[i] = data[i];
        }
        delete[] data;
        data = newData;
        capacity = newCapacity;
    }

    void ensureCapacity(size_t required) {
        if(required <= capacity) return;

        size_t newCapcity = capacity * 2;
        if(required > newCapcity){
            newCapcity = required;
        }
        resize(newCapcity);
    }
public:
    Vector() : data(nullptr), capacity(0), size(0) {
        resize(10);                     //默认初始容量
    }

    explicit Vector(size_t initialCapcity) : data(nullptr), capacity(0), size(0) {
        resize(initialCapcity);
    }

    Vector(std::initializer_list<T> list) : Vector(list.size() + 5) {
        for(const T& value : list) {
            push_back(value);
        }
    }

    ~Vector() {
        delete[] data;
    }

    // 拷贝构造函数
    Vector(const Vector& other) 
        : Vector(other.capacity) {
        size = other.size;
        for (size_t i = 0; i < size; i++) {
            data[i] = other.data[i];
        }
    }
    
    // 拷贝赋值运算符
    Vector& operator=(const Vector& other){
        if(this != &other){
            delete[] data;
            capacity = other.capacity;
            size = other.size();
            data = new T[capacity];
            for(size_t i = 0; i < size; i ++){
                data[i] = other.data[i];
            }
        }
        return *this;
    }
    
    // 移动构造函数
    Vector(Vector&& other) noexcept 
        : data(other.data), capacity(other.capacity), size(other.size) {
        other.data = nullptr;
        other.capacity = 0;
        other.size = 0;
    }
    
    // 移动赋值运算符
    Vector& operator=(Vector&& other) noexcept {
        if (this != &other) {
            delete[] data;
            data = other.data;
            capacity = other.capacity;
            size = other.size;
            
            other.data = nullptr;
            other.capacity = 0;
            other.size = 0;
        }
        return *this;
    }

    void push_back(const T& value) {
        ensureCapacity(size + 1);
        data[size ++] = value;
    }

    void pop_back() {
        if(size == 0){
            throw std::out_of_range("Vector is empty");
        }
        size --;
    }

    void insert(size_t index, const T& value){
        if(index >= size) {
            throw std::out_of_range("Index out of range");
        }
        ensureCapacity(size + 1);

        for(size_t i = size; i > index; i --){
            data[i] = data[i - 1];
        }
        data[index] = value;
        size ++;
    }

    void erase(size_t index) {
        if(index >= capacity){
            throw std::out_of_range("Index out of range");
        }
        for(size_t i = index; i < size - 1; i ++){
            data[i] = data[i + 1];
        }
        size --;
    }

    void clear() {
        size = 0;
    }

    T& operator[](size_t index) {
        if(index >= capacity){
            throw std::out_of_range("Index outof range");
        }
        return data[index];
    }

    const T& operator[](size_t index) const {
        if(index >= capacity){
            throw std::out_of_range("Index outof range");
        }
        return data[index];
    }

    T& at(size_t index) {
        return operator[](index);
    }

    const T& at(size_t index) const {
        return operator[](index);
    }

    // 获取大小
    size_t getSize() const {
        return size;
    }
    
    // 获取容量
    size_t getCapacity() const {
        return capacity;
    }

    bool empty() const {
        return size == 0;
    }

    void print() const {
        for(size_t i = 0; i < size; i ++){
            std::cout << data[i] << " ";
        }
        std::cout << std::endl;
    }
};

}
 

struct Person {
    std::string name;
    int age;
    Person(std::string n = "", int a = 0) : name(std::move(n)), age(a) {}

    friend std::ostream& operator<<(std::ostream& os, const Person& p){
        os << "{" << p.name << ", " << p.age << "}";
        return os;
    }

    bool operator==(const Person& other) const {
        return name == other.name && age == other.age;
    }
};

// 测试用例 - 整数类型
void testIntVector() {
    std::cout << "=== 测试整数类型Vector ===" << std::endl;
    
    Vector<int> vec = {1, 2, 3};
    std::cout << "初始向量: ";
    vec.print(); // 1 2 3
    
    vec.push_back(4);
    vec.push_back(5);
    std::cout << "添加元素后: ";
    vec.print(); // 1 2 3 4 5
    
    vec.insert(2, 10);
    std::cout << "在位置2插入10后: ";
    vec.print(); // 1 2 10 3 4 5
    
    vec.erase(3);
    std::cout << "删除位置3的元素后: ";
    vec.print(); // 1 2 10 4 5
    
    vec.pop_back();
    std::cout << "删除最后一个元素后: ";
    vec.print(); // 1 2 10 4
    
    // 测试拷贝
    Vector<int> copy = vec;
    std::cout << "拷贝的向量: ";
    copy.print(); // 1 2 10 4
    
    // 测试移动
    Vector<int> moved = std::move(vec);
    std::cout << "移动后的向量: ";
    moved.print(); // 1 2 10 4
    std::cout << "原向量大小: " << vec.getSize() << std::endl; // 0
    
    // 测试索引访问
    std::cout << "位置0的元素: " << moved[0] << std::endl; // 1
    moved[0] = 100;
    std::cout << "修改位置0的元素后: " << moved[0] << std::endl; // 100
    std::cout << std::endl;
}

// 测试用例 - 自定义结构体类型
void testPersonVector() {
    std::cout << "=== 测试自定义结构体Person的Vector ===" << std::endl;
    
    // 创建Person向量
    Vector<Person> people;
    people.push_back(Person("Alice", 25));
    people.push_back(Person("Bob", 30));
    people.push_back(Person("Charlie", 35));
    
    std::cout << "初始人员列表: ";
    people.print();
    
    // 插入新人员
    people.insert(1, Person("David", 28));
    std::cout << "插入David后: ";
    people.print();
    
    // 修改元素
    people[0] = Person("Alex", 26);
    std::cout << "修改第一个人后: ";
    people.print();
    
    // 删除元素
    people.erase(2);
    std::cout << "删除第三个人后: ";
    people.print();
    
    // 测试拷贝
    Vector<Person> peopleCopy = people;
    std::cout << "拷贝的人员列表: ";
    peopleCopy.print();
    
    // 测试移动
    Vector<Person> peopleMoved = std::move(people);
    std::cout << "移动后的人员列表: ";
    peopleMoved.print();
    std::cout << "原人员列表大小: " << people.getSize() << std::endl;
    std::cout << std::endl;
}

// 测试用例 - 字符串类型
void testStringVector() {
    std::cout << "=== 测试字符串类型Vector ===" << std::endl;
    
    Vector<std::string> strVec = {"apple", "banana", "cherry"};
    std::cout << "初始字符串向量: ";
    strVec.print();
    
    strVec.push_back("date");
    std::cout << "添加元素后: ";
    strVec.print();
    
    strVec.insert(1, "blueberry");
    std::cout << "插入元素后: ";
    strVec.print();
    
    strVec.erase(3);
    std::cout << "删除元素后: ";
    strVec.print();
    std::cout << std::endl;
}

int main() {
    // 运行各种类型的测试
    testIntVector();
    testPersonVector();
    testStringVector();
    
    return 0;
}
