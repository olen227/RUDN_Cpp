#include <iostream>
using namespace std;

struct Complex {
    double real;
    double imaginary;
};

Complex add(Complex a, Complex b) {
    return {a.real + b.real, a.imaginary + b.imaginary};
}

Complex subtract(Complex a, Complex b) {
    return {a.real - b.real, a.imaginary - b.imaginary};
}

Complex multiply(Complex a, Complex b) {
    return {a.real * b.real - a.imaginary * b.imaginary,
            a.real * b.imaginary + a.imaginary * b.real};
}

void print(Complex c) {
    cout << "(" << c.real;
    if (c.imaginary >= 0) {
        cout << " + " << c.imaginary << "i)";
    } else {
        cout << " - " << -c.imaginary << "i)";
    }
}

int main() {
    int n = 3; 
    Complex numbers[n];

    cout << "=== ВВОД КОМПЛЕКСНЫХ ЧИСЕЛ ===\n";
    for (int i = 0; i < n; i++) {
        cout << "Число " << (i + 1) << ":\n";
        cout << "  Действительная часть: ";
        cin >> numbers[i].real;
        cout << "  Мнимая часть: ";
        cin >> numbers[i].imaginary;
    }

    cout << "\n=== ВЫВОД ===\n";
    for (int i = 0; i < n; i++) {
        cout << "Число " << (i + 1) << ": ";
        print(numbers[i]);
        cout << "\n";
    }

    // Демонстрация операций
    cout << "\n--- Сложение ---\n";
    print(numbers[0]); cout << " + "; print(numbers[1]); cout << " = ";
    print(add(numbers[0], numbers[1])); cout << "\n";

    cout << "--- Вычитание ---\n";
    print(numbers[1]); cout << " - "; print(numbers[2]); cout << " = ";
    print(subtract(numbers[1], numbers[2])); cout << "\n";

    cout << "--- Умножение ---\n";
    print(numbers[0]); cout << " * "; print(numbers[2]); cout << " = ";
    print(multiply(numbers[0], numbers[2])); cout << "\n";

}