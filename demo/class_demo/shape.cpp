#include <iostream>
#include <vector>
#include <memory>
#include <cmath>

//--------------------- 抽象基类 ---------------------
class Shape {
public:
    virtual ~Shape() = default;
    virtual double area() const = 0;
    virtual void draw() const = 0;
    virtual std::string type() const = 0;
};

//--------------------- 二维形状继承体系 ---------------------
class Circle : public Shape {
private:
    double radius;
    static constexpr double PI = 3.141592653589793;

    void validateRadius() const {
        if (radius <= 0) throw std::invalid_argument("Invalid radius");
    }

public:
    explicit Circle(double r) : radius(r) { validateRadius(); }

    double area() const override {
        return PI * radius * radius;
    }

    void draw() const override {
        std::cout << "Drawing Circle (r=" << radius << ")\n";
    }

    std::string type() const override { return "Circle"; }

    // 封装属性访问
    double getRadius() const { return radius; }
    void setRadius(double r) { 
        radius = r;
        validateRadius();
    }
};

class Rectangle : public Shape {
protected:
    double width;
    double height;

    void validateDimensions() const {
        if (width <= 0 || height <= 0)
            throw std::invalid_argument("Invalid dimensions");
    }

public:
    Rectangle(double w, double h) : width(w), height(h) { validateDimensions(); }

    double area() const override {
        return width * height;
    }

    void draw() const override {
        std::cout << "Drawing Rectangle (" << width << "x" << height << ")\n";
    }

    std::string type() const override { return "Rectangle"; }

    // 封装属性访问
    virtual void setDimensions(double w, double h) {
        width = w;
        height = h;
        validateDimensions();
    }

    double getWidth() const { return width; }
    double getHeight() const { return height; }
};

//--------------------- 继承层次扩展 ---------------------
class Square : public Rectangle {
public:
    explicit Square(double side) : Rectangle(side, side) {}

    void setDimensions(double w, double h) override {
        if (w != h) throw std::invalid_argument("Square requires equal sides");
        Rectangle::setDimensions(w, h);
    }

    void setSide(double s) {
        setDimensions(s, s);
    }

    std::string type() const override { 
        return "Square"; 
    }
};

//--------------------- 复合形状 ---------------------
class CompositeShape : public Shape {
    std::vector<std::unique_ptr<Shape>> shapes;

public:
    void addShape(std::unique_ptr<Shape> shape) {
        shapes.push_back(std::move(shape));
    }

    double area() const override {
        double total = 0;
        for (const auto& shape : shapes) {
            total += shape->area();
        }
        return total;
    }

    void draw() const override {
        std::cout << "Drawing CompositeShape containing:\n";
        for (const auto& shape : shapes) {
            std::cout << "- ";
            shape->draw();
        }
    }

    std::string type() const override { 
        return "CompositeShape"; 
    }
};

//最简单的工厂方法实例
class ShapeFactory {
public:
    enum class ShapeType { CIRCLE, RECTANGLE, SQUARE };

    static std::unique_ptr<Shape> createShape(ShapeType type, double arg1, double arg2 = 0) {
        switch (type) {
            case ShapeType::CIRCLE:
                return std::make_unique<Circle>(arg1);
            case ShapeType::RECTANGLE:
                return std::make_unique<Rectangle>(arg1, arg2);
            case ShapeType::SQUARE:
                return std::make_unique<Square>(arg1);
            default:
                throw std::invalid_argument("Unknown shape type");
        }
    }
};


class DrawingManager {
    std::vector<std::unique_ptr<Shape>> shapes;
public:
    void addShape(std::unique_ptr<Shape> shape){
        shapes.push_back(std::move(shape));
    }

    void drawAll() const {
        std::cout << "\n=== Drawing All Shapes === \n" ;
        for(const auto& shape : shapes){
            shape->draw();
            std::cout << "Area : " << shape->area() << "\n\n"; 
        }
    }
};


int main()
{
    DrawingManager manager;
    auto circle = ShapeFactory::createShape(
        ShapeFactory::ShapeType::CIRCLE, 5.0
    );
    manager.addShape(std::move(circle));
    manager.drawAll();

    return 0;
}