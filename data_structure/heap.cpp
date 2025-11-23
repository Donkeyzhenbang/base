#include <iostream>
#include <vector>
#include <functional>
#include <stdexcept>

/**
 * @brief 基于动态数组的堆实现
 * !完全二叉树的存储使用vector容器优于链表 紧凑排列 紧凑排列 紧凑排列！！！
 * 父节点 (i - 1) / 2 
 * 左子节点 2i + 1 
 * 右子节点 2i + 2
 * vector pop_back他们出最后一个并用最后一个覆盖堆顶即相当于弹出堆顶
 * vector size元素个数
 * !std::less<T> 的逻辑是：a < b 时返回 true（即 “b 比 a 大”）
 * 大顶堆 comp(a, b) 相当于 a < b 如果data_[selected] < data_[left] 说明左子节点更大也即优先级更高。则需要更新selected
 * left < size 确定不会越界访问
 *! explicit Heap(int initialCapacity = 10, const Compare& comp = Compare()) : comp_(comp){ 注意这里初始化方式
 */
template<typename T, typename Compare = std::less<T>>
class Heap{
private:
    std::vector<T> data_;    //底层容器
    Compare comp_;          //比较器实例
    
    // 向上堆化 : 插入元素后维持堆性质
    void heapifyUp(int index) {
        if(index == 0) return;
        int parent = (index - 1) / 2; //父节点索引 完全二叉树

        // 比较当前节点与父节点，根据比较器决定是否交换
        // 例：std::less<T> 时，若 data_[index] > data_[parent] → 交换（大顶堆）
        // 例：std::greater<T> 时，若 data_[index] < data_[parent] → 交换（小顶堆）
        if(comp_(data_[parent], data_[index])) {
            std::swap(data_[index], data_[parent]);
            heapifyUp(parent);  //递归向上调整
        }
    }

    // 向下堆化 : 删除堆顶后维持堆性质
    void heapifyDown(int index) {
        int size = data_.size();
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        int selected = index; // 记录需要交换的节点（当前节点/左子/右子）

        if(left < size && comp_(data_[selected], data_[left])) {
            selected = left;
        }

        if(right < size && comp_(data_[selected], data_[right])) {
            selected = right;
        }
        if(selected != index){
            std::swap(data_[index], data_[selected]);
            heapifyDown(selected);
        }
    }
public:
    // 构造函数（默认小顶堆，可指定初始容量提升效率）
    explicit Heap(int initialCapacity = 10, const Compare& comp = Compare()) : comp_(comp){
        data_.reserve(initialCapacity); // 预分配容量，优化性能
    }

    //析构函数
    ~Heap() = default; //delete vector会自动释放内存

    //拷贝构造函数
    Heap(const Heap& othor) = default;

    Heap& operator=(const Heap&) = default;

    void push(const T& value) {
        data_.push_back(value);
        heapifyUp(data_.size() - 1);
    }

    //移动版本push 优化右值元素的插入性能
    void push(T&& value) {
        data_.push_back(std::move(value));
        heapifyUp(data_.size() - 1);
    }

    void pop() {
        if (empty()) {
            throw std::out_of_range("Heap is empty: cannot get top");
        }
        data_[0] = std::move(data_.back());
        data_.pop_back();
        if(!empty()){
            heapifyDown(0);
        }
    }

    const T& top() const {
        if (empty()) {
            throw std::out_of_range("Heap is empty: cannot get top");
        }
        return data_[0];
    }

    bool empty() const {
        return data_.empty();
    }

    size_t size() const {
        return data_.size();
    }

    void clear() {
        data_.clear();
    }
    // 打印堆内容（要求 T 支持 << 运算符，或自定义打印逻辑）
    void print(const std::string& name = "") const {
        if (!name.empty()) {
            std::cout << name << ": ";
        }
        for (const auto& elem : data_) {
            std::cout << elem << " ";
        }
        std::cout << "(size: " << size() << ")" << std::endl;
    }

};


// --------------------------
// 测试用例：验证模板堆的泛化能力
// --------------------------

// 1. 测试基本类型（int、double）
void testBasicTypes() {
    std::cout << "===== Test Basic Types =====" << std::endl;

    // 测试 大顶堆（默认比较器 std::less<int>）
    Heap<int> maxHeap;
    maxHeap.push(10);
    maxHeap.push(5);
    maxHeap.push(20);
    maxHeap.push(3);
    maxHeap.push(8);
    maxHeap.print("Int Max Heap");  // 堆顶是 20，内容类似：20 10 5 3 8
    std::cout << "Top: " << maxHeap.top() << std::endl;  // 20

    // 测试 小顶堆（指定比较器 std::greater<int>）
    Heap<int, std::greater<int>> minHeap;
    minHeap.push(10);
    minHeap.push(5);
    minHeap.push(20);
    minHeap.push(3);
    minHeap.push(8);
    minHeap.print("Int Min Heap");  // 堆顶是 3，内容类似：3 5 20 10 8
    std::cout << "Top: " << minHeap.top() << std::endl;  // 3

    // 测试 double 类型
    Heap<double, std::greater<double>> doubleMinHeap;
    doubleMinHeap.push(3.14);
    doubleMinHeap.push(1.59);
    doubleMinHeap.push(2.65);
    doubleMinHeap.print("Double Min Heap");  // 1.59 3.14 2.65
    std::cout << std::endl;
}

// 2. 测试自定义结构体（需自定义比较器）
struct Person {
    std::string name;
    int age;

    // 支持 << 运算符，用于 print 函数
    friend std::ostream& operator<<(std::ostream& os, const Person& p) {
        os << p.name << "(" << p.age << ")";
        return os;
    }
};

// 自定义比较器：按年龄升序（小顶堆）
struct ComparePersonByAgeAsc {
    bool operator()(const Person& a, const Person& b) const {
        return a.age > b.age;  // 注意：比较器返回 true 表示 a 应排在 b 后面（适配堆化逻辑）
    }
};

// 自定义比较器：按姓名字典序降序（大顶堆）
struct ComparePersonByNameDesc {
    bool operator()(const Person& a, const Person& b) const {
        return a.name < b.name;  // 姓名字典序：a.name < b.name → a 优先级低，排在后面
    }
};

void testCustomStruct() {
    std::cout << "===== Test Custom Struct (Person) =====" << std::endl;

    // 按年龄升序的小顶堆
    Heap<Person, ComparePersonByAgeAsc> ageMinHeap;
    ageMinHeap.push({"Alice", 30});
    ageMinHeap.push({"Bob", 25});
    ageMinHeap.push({"Charlie", 35});
    ageMinHeap.print("Age Min Heap");  // Bob(25) Alice(30) Charlie(35)
    std::cout << "Top (Youngest): " << ageMinHeap.top() << std::endl;  // Bob(25)

    // 按姓名字典序降序的大顶堆
    Heap<Person, ComparePersonByNameDesc> nameMaxHeap;
    nameMaxHeap.push({"Alice", 30});
    nameMaxHeap.push({"Bob", 25});
    nameMaxHeap.push({"Charlie", 35});
    nameMaxHeap.print("Name Max Heap");  // Charlie(35) Bob(25) Alice(30)
    std::cout << "Top (Last Name): " << nameMaxHeap.top() << std::endl;  // Charlie(35)
    std::cout << std::endl;
}

// 3. 测试移动语义（优化性能）
void testMoveSemantics() {
    std::cout << "===== Test Move Semantics =====" << std::endl;

    Heap<std::string> strHeap;
    std::string temp = "Hello";
    strHeap.push(temp);                // 拷贝插入
    strHeap.push(std::move(temp));     // 移动插入（temp 此后失效）
    strHeap.push("World");             // 直接插入临时对象（自动移动）
    strHeap.print("String Heap");      // World Hello Hello（大顶堆，堆顶是 "World"）
    std::cout << std::endl;
}

int main() {
    testBasicTypes();
    testCustomStruct();
    testMoveSemantics();
    return 0;
}