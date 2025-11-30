#include <iostream>
using namespace std;

void swapByValue(int a, int b) {
    cout << "  [В функции swapByValue до обмена] a = " << a << ", b = " << b << endl;
    
    int temp = a;
    a = b;
    b = temp;
    
    cout << "  [В функции swapByValue после обмена] a = " << a << ", b = " << b << "\n";
}

void swapByPointer(int* a, int* b) {
    if (a == nullptr || b == nullptr) {
        cerr << "Ошибка: указатель nullptr\n";
        return;
    }
    
    cout << "  [В функции swapByPointer до обмена] *a = " << *a << ", *b = " << *b << "\n";
    
    int temp = *a;
    *a = *b;
    *b = temp;
    
    cout << "  [В функции swapByPointer после обмена] *a = " << *a << ", *b = " << *b << "\n";
}

void swapByReference(int& a, int& b) {
    cout << "  [В функции swapByReference до обмена] a = " << a << ", b = " << b << "\n";
    
    int temp = a;
    a = b;
    b = temp;
    
    cout << "  [В функции swapByReference после обмена] a = " << a << ", b = " << b << "\n";
}

int main() {
    cout << "=== ДЕМОНСТРАЦИЯ РАЗНЫХ СПОСОБОВ ПЕРЕДАЧИ ПАРАМЕТРОВ ===\n\n";
    
    int x1 = 10, y1 = 20;
    cout << "--- ТЕСТ 1: Передача по значению ---\n";
    cout << "До вызова функции: x1 = " << x1 << ", y1 = " << y1 << "\n";
    swapByValue(x1, y1);
    cout << "После вызова функции: x1 = " << x1 << ", y1 = " << y1 << "\n";
    cout << "Обмен НЕ произошел! Функция работала с копиями.\n\n";
    
    int x2 = 10, y2 = 20;
    cout << "--- ТЕСТ 2: Передача по указателю ---\n";
    cout << "До вызова функции: x2 = " << x2 << ", y2 = " << y2 << "\n";
    swapByPointer(&x2, &y2);
    cout << "После вызова функции: x2 = " << x2 << ", y2 = " << y2 << "\n";
    cout << "Обмен произошел! Функция изменила оригинальные значения.\n\n";
    
    int x3 = 10, y3 = 20;
    cout << "--- ТЕСТ 3: Передача по ссылке ---\n";
    cout << "До вызова функции: x3 = " << x3 << ", y3 = " << y3 << "\n";
    swapByReference(x3, y3);
    cout << "После вызова функции: x3 = " << x3 << ", y3 = " << y3 << "\n";
    cout << "Обмен произошел! Ссылки работают как алиасы переменных.\n\n";
    
    cout << "=== ВЫВОД ===\n";
    cout << "По значению: копируется значение, изменения не влияют на оригинал\n";
    cout << "По указателю: передается адрес, можно изменить оригинал через *\n";
    cout << "По ссылке: работает как алиас, синтаксис проще, но тоже меняет оригинал\n";
    
    return 0;
}
