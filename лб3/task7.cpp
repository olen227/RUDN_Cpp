#include <iostream>
#include <string>
using namespace std;

class Complex {
private:
    double real;
    double imaginary;

public:
    Complex(double r = 0, double i = 0) : real(r), imaginary(i) {}
    
    // TODO: Геттеры и сеттеры
    
    // Геттер для получения действительной части комплексного числа
    double getReal() const {
        return real;
    }
    
    // Геттер для получения мнимой части комплексного числа
    double getImaginary() const {
        return imaginary;
    }
    
    // Сеттер для установки действительной части
    void setReal(double r) {
        real = r;
    }
    
    // Сеттер для установки мнимой части
    void setImaginary(double i) {
        imaginary = i;
    }
    
    // TODO: Перегрузить операторы:
    // - Complex operator+(const Complex& other) const
    
    // Перегрузка оператора сложения для комплексных чисел
    Complex operator+(const Complex& other) const {
        // Сложение: (a + bi) + (c + di) = (a + c) + (b + d)i
        return Complex(real + other.real, imaginary + other.imaginary);
    }
    
    // - Complex operator-(const Complex& other) const
    
    // Перегрузка оператора вычитания для комплексных чисел
    Complex operator-(const Complex& other) const {
        // Вычитание: (a + bi) - (c + di) = (a - c) + (b - d)i
        return Complex(real - other.real, imaginary - other.imaginary);
    }
    
    // - Complex operator*(const Complex& other) const
    
    // Перегрузка оператора умножения для комплексных чисел
    Complex operator*(const Complex& other) const {
        // Умножение: (a + bi) * (c + di) = (ac - bd) + (ad + bc)i
        double r = real * other.real - imaginary * other.imaginary;      // Действительная часть
        double i = real * other.imaginary + imaginary * other.real;      // Мнимая часть
        return Complex(r, i);
    }
    
    // - bool operator==(const Complex& other) const
    
    // Перегрузка оператора сравнения для комплексных чисел
    bool operator==(const Complex& other) const {
        // Два комплексных числа равны, если равны их действительные и мнимые части
        return (real == other.real && imaginary == other.imaginary);
    }
    
    // TODO: Сделать дружественными:
    // - friend ostream& operator<<(ostream& os, const Complex& c)
    
    // Дружественная функция для перегрузки оператора вывода
    friend ostream& operator<<(ostream& os, const Complex& c) {
        // Форматированный вывод комплексного числа
        if (c.imaginary >= 0) {
            os << c.real << " + " << c.imaginary << "i";
        } else {
            os << c.real << " - " << (-c.imaginary) << "i";
        }
        return os;
    }
    
    // - friend istream& operator>>(istream& is, Complex& c)
    
    // Дружественная функция для перегрузки оператора ввода
    friend istream& operator>>(istream& is, Complex& c) {
        cout << "Введите действительную часть: ";
        is >> c.real;      // Ввод действительной части
        cout << "Введите мнимую часть: ";
        is >> c.imaginary; // Ввод мнимой части
        return is;
    }
    
    void display() const {
        cout << real << " + " << imaginary << "i" << endl;
    }
};

int main() {
    // TODO: Создать несколько комплексных чисел
    // Продемонстрировать работу перегруженных операторов
    // Показать ввод/вывод через перегруженные операторы << и >>
    
    cout << "=== Создание комплексных чисел ===" << endl;
    // Создание комплексных чисел
    Complex c1(3.0, 4.0);
    Complex c2(1.0, 2.0);
    Complex c3(3.0, 4.0);
    
    // Демонстрация перегруженного оператора вывода <<
    cout << "c1 = " << c1 << endl;
    cout << "c2 = " << c2 << endl;
    cout << "c3 = " << c3 << endl;
    cout << endl;
    
    cout << "=== Арифметические операции ===" << endl;
    // Демонстрация перегруженного оператора сложения
    Complex sum = c1 + c2;
    cout << "c1 + c2 = " << sum << endl;
    
    // Демонстрация перегруженного оператора вычитания
    Complex diff = c1 - c2;
    cout << "c1 - c2 = " << diff << endl;
    
    // Демонстрация перегруженного оператора умножения
    Complex prod = c1 * c2;
    cout << "c1 * c2 = " << prod << endl;
    cout << endl;
    
    cout << "=== Операция сравнения ===" << endl;
    // Демонстрация перегруженного оператора сравнения
    if (c1 == c3) {
        cout << "c1 и c3 равны" << endl;
    } else {
        cout << "c1 и c3 не равны" << endl;
    }
    
    if (c1 == c2) {
        cout << "c1 и c2 равны" << endl;
    } else {
        cout << "c1 и c2 не равны" << endl;
    }
    cout << endl;
    
    cout << "=== Ввод комплексного числа ===" << endl;
    Complex c4;
    // Использование сеттеров для установки значений (альтернатива вводу)
    c4.setReal(5.0);
    c4.setImaginary(-3.0);
    cout << "c4 = " << c4 << endl;
    cout << endl;
    
    cout << "=== Сложные выражения ===" << endl;
    // Демонстрация использования перегруженных операторов в сложных выражениях
    Complex result = (c1 + c2) * c4;
    cout << "(c1 + c2) * c4 = " << result << endl;
}