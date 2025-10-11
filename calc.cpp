#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace std;

int main() {
    setlocale(LC_ALL, "ru");

    char a1;
    int x = 0, y = 0, R = 0;

    srand(time(nullptr)); // инициализация генератора случайных чисел
    int random = 1 + rand() % 10;

    cout << "Введите первое число: ";
    cin >> x;
    cout << "Введите второе число: ";
    cin >> y;
    cout << "Введите знак (+, -, *, /, ?, r): ";
    cin >> a1;

    switch (a1) {
        case '+':
            R = x + y;
            cout << "Результат: " << R;
            break;

        case '-':
            R = x - y;
            cout << "Результат: " << R;
            break;

        case '*':
            R = x * y;
            cout << "Результат: " << R;
            break;

        case '/':
            if (y != 0) {
                R = x / y;
                cout << "Результат: " << R;
            } else {
                cout << "Ошибка: деление на ноль!";
            }
            break;

        case '?':
            // пример цикла с условием (иначе он будет бесконечным)
            while (x <= 100) {
                cout << x << " + " << y << " = ?" << endl;
                break; // чтобы не зациклиться
            }
            break;

        case 'r':
            cout << "Случайное число: " << random << endl;
            if (random == x || random == y) {
                cout << "RIGHT!!!";
            } else {
                cout << "Nah...";
            }
            break;

        default:
            cout << "Неизвестная операция";
    }

    cout << endl;
    return 0;
}   