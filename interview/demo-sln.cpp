#include <iostream>
#include <string>
#include <cstring>

using namespace std;

class Expr {
public:
    virtual std::string toString() const = 0;
};

class Var : public Expr {   //元素
public:
    std::string name;
    Var(const std::string& name) : name(name) {}
    std::string toString() const override {
        return name;
    }
};

class Add : public Expr {
public:
    Expr* left;
    Expr* right;
    Add(Expr* left, Expr* right) : left(left), right(right) {}
    std::string toString() const override {
        string s = "";
        // s = dynamic_cast<Var*>(left)->name + " " + "+" + " " + dynamic_cast<Var*>(right)->name;
        if(dynamic_cast<Add*>(right)) //检索加号
            s = left->toString() +" " + "+" + " " + "(" + right->toString() + ")";
        else
            s = left->toString() + " " + "+" + " " + right->toString();

        return s;
    }
};

class Mult : public Expr {
public:
    Expr* left;
    Expr* right;
    Mult(Expr* left, Expr* right) : left(left), right(right) {}
    std::string toString() const override {
        string s = "";
        if(dynamic_cast<Add*>(left)) //检索加号
            s = "(" + left->toString() + ")" +" " + "*" + " " + right->toString();
        else
            s = left->toString() + " " + "*" + " " + right->toString();
        return s;
    }
};

int main() {
    Expr* x = new Var("x");
    Expr* y = new Var("y");
    Expr* z = new Var("z");

    Expr* expr1 = new Add(x, new Mult(y, z));
    std::cout << expr1->toString() << std::endl; // x + y * z


    Expr* expr2 = new Mult(new Add(x, y), z);
    std::cout << expr2->toString() << std::endl; // (x + y) * z

    Expr* expr3 = new Add(new Add(x, y), z);
    std::cout << expr3->toString() << std::endl; // x + y + z

    Expr* expr4 = new Add(x, new Add(y, z));
    std::cout << expr4->toString() << std::endl; // x + (y + z)

    return 0;

}

// if (dynamic_cast<Add*>(left)) {
//   ...
// }