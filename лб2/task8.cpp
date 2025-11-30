#include <iostream>
using namespace std;

void arrayInfo(int* arr, int size) {
    if (size <= 0) {
        cerr << "Ошибка: размер массива должен быть положительным\n";
        return;
    }
    
    if (arr == nullptr) {
        cerr << "Ошибка: массив не инициализирован (nullptr)\n";
        return;
    }
    
    cout << "\n--- Информация о массиве ---\n";
    cout << "Размер массива: " << size << " элементов\n";
    
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += *(arr + i);
    }
    cout << "Сумма элементов: " << sum << "\n";
    
    cout << "Первый элемент: " << *arr << "\n";
    cout << "Последний элемент: " << *(arr + size - 1) << "\n";
    
    double average = static_cast<double>(sum) / size;
    cout << "Среднее значение: " << average << "\n";
    
    cout << "Элементы: ";
    for (int i = 0; i < size; i++) {
        cout << *(arr + i);
        if (i < size - 1) cout << ", ";
    }
    cout << "\n";
}

int main() {
    cout << "=== ДЕМОНСТРАЦИЯ ФУНКЦИИ arrayInfo ===\n";
    
    int arr1[] = {10, 20, 30, 40, 50};
    int size1 = sizeof(arr1) / sizeof(arr1[0]);
    
    cout << "\n=== МАССИВ 1 ===";
    arrayInfo(arr1, size1);
    
    int arr2[] = {5, -3, 8, 12, -7, 15, 0, 9};
    int size2 = sizeof(arr2) / sizeof(arr2[0]);
    
    cout << "\n=== МАССИВ 2 ===";
    arrayInfo(arr2, size2);
    
    cout << "\n=== ДИНАМИЧЕСКИЙ МАССИВ ===";
    int dynamicSize = 6;
    int* dynamicArr = new int[dynamicSize];
    
    for (int i = 0; i < dynamicSize; i++) {
        dynamicArr[i] = i * i;
    }
    
    arrayInfo(dynamicArr, dynamicSize);
    
    delete[] dynamicArr;
    
    cout << "\n=== ТЕСТ ЗАЩИТЫ ОТ ОШИБОК ===\n";
    cout << "Попытка вызова с нулевым размером:\n";
    arrayInfo(arr1, 0);
    
    cout << "\nПопытка вызова с nullptr:\n";
    arrayInfo(nullptr, 5);
    
    return 0;
}
