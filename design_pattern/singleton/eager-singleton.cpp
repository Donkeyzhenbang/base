#include <iostream>

//! 饿汉式 : 程序启动就初始化，线程安全 
//! 懒汉式 : 程序第一次被使用时才初始化，需要注意线程安全
//! 在C++中，类的静态成员变量需要在类外进行定义（初始化），否则编译器不会为它分配内存。
// 饿汉式
template <typename T>
class EagerSingleton {
private:

    static T* instance_;    //删除拷贝构造与赋值操作
    EagerSingleton() = default;
    ~EagerSingleton() = default;
    EagerSingleton(const EagerSingleton&) = delete;
    EagerSingleton& operator=(const EagerSingleton&) = delete;
public:
    static T* GetInstance() {
        return instance_;
    }

    static void DestroyInstance() {
        if(instance_ != nullptr) {
            delete instance_;       //仅释放内存，但是指针扔指向堆上动态分配的内存块
            instance_ = nullptr;    //如果不置为nullptr，会导致野指针行为，野指针指向的内存已被释放但是其值仍是地址未改变，后续如解引用 *instance_、再次 delete instance_ 等），会导致未定义行为（程序崩溃、数据错乱
                                    
        }
    }

};

// 静态成员初始化（在main函数之前创建）
template<typename T>
T* EagerSingleton<T>::instance_ = new T();

class DatabaseManager {
private:
    std::string connection_string_;
public:
    DatabaseManager() : connection_string_("default_connection") {
        std::cout << "DatabaseManager constructed (Eager)" << std::endl;
    }
    
    ~DatabaseManager() {
        std::cout << "DatabaseManager destroyed" << std::endl;
    }
    void connect(const std::string& conn_str) { //避免拷贝+禁止修改
        connection_string_ = conn_str;
        std::cout << "Connected to : " << conn_str << std::endl;
    }
    void executeQuery(const std::string& query){
        std::cout << "Executing query : " << query << std::endl;
    }
    std::string getConnectionString() const {
        return connection_string_;
    }


};

void testEagerSingleton(){
    std::cout << "=== Testing Eager Singleton ===" << std::endl;

    DatabaseManager* db1 = EagerSingleton<DatabaseManager>::GetInstance();
    DatabaseManager* db2 = EagerSingleton<DatabaseManager>::GetInstance();
    std::cout << "db1 : address : " << db1 << std::endl;
    std::cout << "db2 : address : " << db2 << std::endl;
    std::cout << "Same instance : " << (db1 == db2) << std::endl;
    db1->connect("Server = localhost, database = test");
    db1->executeQuery("Select * FROM users");
    std::cout << "Connection string : " << db1->getConnectionString() << std::endl;
}

int main()
{
    testEagerSingleton();
    return 0;
}
 