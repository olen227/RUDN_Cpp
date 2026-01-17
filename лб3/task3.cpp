#include <iostream>
#include <vector>
#include <cmath>
using namespace std;

class Shape {
protected:
    string color;
    string name;

public:
    Shape(string n, string c) : name(n), color(c) {}
    virtual ~Shape() {}
    
    // TODO: Объявить чисто виртуальные методы:
    // - calculateArea()
    // - calculatePerimeter()
    // - draw()
    
    // Чисто виртуальный метод для вычисления площади фигуры
    virtual double calculateArea() const = 0;
    
    // Чисто виртуальный метод для вычисления периметра фигуры
    virtual double calculatePerimeter() const = 0;
    
    // Чисто виртуальный метод для отрисовки фигуры
    virtual void draw() const = 0;
    
    virtual void displayInfo() const {
        cout << "Фигура: " << name << ", Цвет: " << color << endl;
    }
};

class Circle : public Shape {
private:
    double radius;

public:
    Circle(string c, double r) : Shape("Круг", c), radius(r) {}
    
    // TODO: Реализовать виртуальные методы
    // Площадь круга: π * r²
    // Периметр круга: 2 * π * r
    
    // Переопределение метода для вычисления площади круга по формуле π * r²
    double calculateArea() const override {
        return M_PI * radius * radius;
    }
    
    // Переопределение метода для вычисления периметра (длины окружности) по формуле 2 * π * r
    double calculatePerimeter() const override {
        return 2 * M_PI * radius;
    }
    
    // Переопределение метода для отрисовки круга
    void draw() const override {
        cout << "Рисуем круг радиусом " << radius << endl;
    }
};

class Rectangle : public Shape {
private:
    double width, height;

public:
    Rectangle(string c, double w, double h) : Shape("Прямоугольник", c), width(w), height(h) {}
    
    // TODO: Реализовать виртуальные методы
    
    // Переопределение метода для вычисления площади прямоугольника (ширина * высота)
    double calculateArea() const override {
        return width * height;
    }
    
    // Переопределение метода для вычисления периметра прямоугольника (2 * (ширина + высота))
    double calculatePerimeter() const override {
        return 2 * (width + height);
    }
    
    // Переопределение метода для отрисовки прямоугольника
    void draw() const override {
        cout << "Рисуем прямоугольник " << width << "x" << height << endl;
    }
};

class Triangle : public Shape {
private:
    double sideA, sideB, sideC;

public:
    Triangle(string c, double a, double b, double cc) : Shape("Треугольник", c), sideA(a), sideB(b), sideC(cc) {}
    
    // TODO: Реализовать виртуальные методы
    // Площадь по формуле Герона
    
    // Переопределение метода для вычисления площади треугольника по формуле Герона
    double calculateArea() const override {
        double s = (sideA + sideB + sideC) / 2.0;  // Вычисление полупериметра
        // Формула Герона: √(s * (s - a) * (s - b) * (s - c))
        return sqrt(s * (s - sideA) * (s - sideB) * (s - sideC));
    }
    
    // Переопределение метода для вычисления периметра треугольника (сумма всех сторон)
    double calculatePerimeter() const override {
        return sideA + sideB + sideC;
    }
    
    // Переопределение метода для отрисовки треугольника
    void draw() const override {
        cout << "Рисуем треугольник со сторонами " << sideA << ", " << sideB << ", " << sideC << endl;
    }
};

int main() {
    vector<Shape*> shapes;
    shapes.push_back(new Circle("Красный", 5.0));
    shapes.push_back(new Rectangle("Синий", 4.0, 6.0));
    shapes.push_back(new Triangle("Зеленый", 3.0, 4.0, 5.0));
    
    // TODO: Продемонстрировать полиморфизм - вывести информацию
    // о всех фигурах через указатель на базовый класс
    
    cout << "=== Демонстрация полиморфизма ===" << endl << endl;
    
    // Цикл для демонстрации полиморфного поведения
    for (auto shape : shapes) {
        shape->displayInfo();              // Вызов виртуального метода displayInfo()
        shape->draw();                     // Вызов виртуального метода draw()
        cout << "Площадь: " << shape->calculateArea() << endl;         // Вызов виртуального метода calculateArea()
        cout << "Периметр: " << shape->calculatePerimeter() << endl;   // Вызов виртуального метода calculatePerimeter()
        cout << endl;
    }
    
    // Очистка памяти
    for (auto shape : shapes) {
        delete shape;
    }

}