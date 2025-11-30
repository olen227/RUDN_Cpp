#include <iostream>
using namespace std;

void fillFibonacci(int* arr, int size) {
    if (size <= 0) return;
    
    if (size >= 1) {
        *arr = 0;
    }
    
    if (size >= 2) {
        *(arr + 1) = 1;
    }
    
    int* current = arr + 2;
    int* prev1 = arr + 1;
    int* prev2 = arr;
    
    for (int i = 2; i < size; i++) {
        *current = *prev1 + *prev2;
        current++;
        prev1++;
        prev2++;
    }
}

int sumArray(int* arr, int size) {
    int sum = 0;
    int* ptr = arr;
    
    for (int i = 0; i < size; i++) {
        sum += *ptr;
        ptr++;
    }
    
    return sum;
}

int findMin(int* arr, int size) {
    if (size <= 0) {
        cerr << "Ошибка: пустой массив\n";
        return 0;
    }
    
    int min = *arr;
    int* ptr = arr + 1;
    
    for (int i = 1; i < size; i++) {
        if (*ptr < min) {
            min = *ptr;
        }
        ptr++;
    }
    
    return min;
}

void reverseCopy(int* source, int* dest, int size) {
    int* srcPtr = source;
    int* destPtr = dest + size - 1;
    
    for (int i = 0; i < size; i++) {
        *destPtr = *srcPtr;
        srcPtr++;
        destPtr--;
    }
}

void printArray(int* arr, int size) {
    int* ptr = arr;
    for (int i = 0; i < size; i++) {
        cout << *ptr << " ";
        ptr++;
    }
    cout << "\n";
}

int main() {
    const int SIZE = 10;
    int fibonacci[SIZE];
    int reversed[SIZE];
    
    cout << "=== РАБОТА С МАССИВАМИ ЧЕРЕЗ УКАЗАТЕЛИ ===\n\n";
    
    cout << "1. Заполнение массива последовательностью Фибоначчи:\n";
    fillFibonacci(fibonacci, SIZE);
    cout << "   ";
    printArray(fibonacci, SIZE);
    
    cout << "\n2. Сумма всех элементов:\n";
    int sum = sumArray(fibonacci, SIZE);
    cout << "   Сумма = " << sum << "\n";
    
    cout << "\n3. Минимальный элемент:\n";
    int minValue = findMin(fibonacci, SIZE);
    cout << "   Минимум = " << minValue << "\n";
    
    cout << "\n4. Копирование в обратном порядке:\n";
    reverseCopy(fibonacci, reversed, SIZE);
    cout << "   Оригинал:  ";
    printArray(fibonacci, SIZE);
    cout << "   Обратный:  ";
    printArray(reversed, SIZE);
    
    return 0;
}
