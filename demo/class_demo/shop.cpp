#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <ctime>
#include <stdexcept>
#include <iomanip>

//--------------------- 基础类型定义 ---------------------
using UserID = std::string;
using ProductID = int;
using Timestamp = std::time_t;

//--------------------- 地址结构体 ---------------------
struct Address {
    std::string street;
    std::string city;
    std::string state;
    std::string zip;

    void print() const {
        std::cout << street << ", " << city << ", " << state << " " << zip;
    }
};

//--------------------- 抽象商品类 ---------------------
class Product {
protected:
    ProductID id;
    std::string name;
    double price;
    int stock;

public:
    Product(ProductID id, std::string name, double price, int stock)
        : id(id), name(std::move(name)), price(price), stock(stock) {}

    virtual ~Product() = default;

    // 通用接口
    virtual void display() const = 0;
    virtual std::string getCategory() const = 0;

    // 通用方法
    bool deductStock(int quantity) {
        if (stock >= quantity) {
            stock -= quantity;
            return true;
        }
        return false;
    }

    void restock(int amount) { stock += amount; }

    // Getter方法
    ProductID getId() const { return id; }
    double getPrice() const { return price; }
    int getStock() const { return stock; }
    const std::string& getName() const { return name; }
};

//--------------------- 具体商品类型 ---------------------
class Electronics : public Product {
    std::string brand;
    std::string warranty;

public:
    Electronics(ProductID id, std::string name, double price, int stock,
                std::string brand, std::string warranty)
        : Product(id, std::move(name), price, stock), //通过基类初始化
          brand(std::move(brand)), warranty(std::move(warranty)) {}

    void display() const override {
        std::cout << "[Electronics] " << name << " (" << brand << ")\n"
                  << "Price: $" << price << " | Stock: " << stock << "\n"
                  << "Warranty: " << warranty << "\n";
    }

    std::string getCategory() const override { return "Electronics"; }
};

class Book : public Product {
    std::string author;
    std::string isbn;

public:
    Book(ProductID id, std::string name, double price, int stock,
         std::string author, std::string isbn)
        : Product(id, std::move(name), price, stock),
          author(std::move(author)), isbn(std::move(isbn)) {}

    void display() const override {
        std::cout << "[Book] " << name << " by " << author << "\n"
                  << "ISBN: " << isbn << " | Price: $" << price << "\n"
                  << "Stock: " << stock << "\n";
    }

    std::string getCategory() const override { return "Books"; }
};

//--------------------- 用户类 ---------------------
class User {
private:
    UserID id;
    std::string password;
    std::vector<Address> addresses;
    std::map<Timestamp, double> orderHistory;

public:
    User(UserID id, std::string pwd)
        : id(std::move(id)), password(std::move(pwd)) {}

    void addAddress(const Address& addr) {
        addresses.push_back(addr);
    }

    void addOrder(Timestamp ts, double amount) {
        orderHistory[ts] = amount;
    }

    void printProfile() const {
        std::cout << "User ID: " << id << "\nAddresses:\n";
        for (const auto& addr : addresses) {
            addr.print();
            std::cout << "\n";
        }
    }

    // Getter方法
    const UserID& getId() const { return id; }
    const std::string& getPassword() const { return password; }
};

class ShoppingCart {
private:
    std::unordered_map<ProductID, int> items;
    const User& owner;
public:
    explicit ShoppingCart(const User& user) : owner(user) {}
    void addItem(Product& product, int quantity){
        if(product.deductStock(quantity)){
            items[product.getId()] += quantity;
        } else {
            throw std::runtime_error("Insufficient stock for " + product.getName());
        }
    }

    void removeItem(ProductID pid, int quantity){
        if(items[pid] >= quantity){
            items[pid] -= quantity;
        }else {
            items.erase(pid);
        }
    }

    double calculateTotal(const std::map<ProductID, std::unique_ptr<Product>>& catalog) const {
        double total = 0;
        for(const auto& [pid, qty] : items){
            if(catalog.count(pid)){
                total += catalog.at(pid)->getPrice() * qty;
            }
        }
        return total;
    }

};

class OrderSystem {
private:
    std::map<ProductID, std::unique_ptr<Product>>& catalog;
    std::unordered_map<UserID, std::unique_ptr<User>> users;
    std::unordered_map<UserID, ShoppingCart> carts;

public:
    void registerUser(const UserID& id, const std::string& password){

    }

    User& authenticate(){

    }

    void addToCart(const UserID& uid, ProductID pid, int qty){

    }
}

int main()
{
    OrderSystem system;

    system.addProduct(std::make_unique<Electronics>(
        1001, "Smartphone X", 599.99, 50,
        "TechBrand", "2 years"
    ));
    system.addProduct(std::make_unique<Book>(
        2001, "C++ Programming", 49.99, 100,
        "Bjarne Stroustrup", "978-0321563842"
    ));

    system.registerUser("user1", "password123");
    system.registerUser("user2", "abc@123");

    User& user = system.authenticate("user1", "password123");
    user.addAddress({"123 Main St", "New York", "NY", "10001"});

    // 购物操作
    system.addToCart("user1", 1001, 2);
    system.addToCart("user1", 2001, 1);


    // 查看购物车
    system.displayProducts();
    system.checkout("user1", user.getAddresses().front());

}