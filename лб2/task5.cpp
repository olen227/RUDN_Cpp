#include <iostream>
#include <string>
#include <limits>
using namespace std;

struct Book {
    string title;
    string author;
    int year;
    double price;
};

void printBooks(Book* books, int size) {
    if (books == nullptr || size <= 0) {
        cerr << "Ошибка: некорректные параметры\n";
        return;
    }
    
    cout << "\n=== СПИСОК КНИГ ===\n";
    for (int i = 0; i < size; i++) {
        cout << "\nКнига " << (i + 1) << ":\n";
        cout << "  Название: " << books[i].title << "\n";
        cout << "  Автор: " << books[i].author << "\n";
        cout << "  Год издания: " << books[i].year << "\n";
        cout << "  Цена: " << books[i].price << " руб.\n";
    }
}

void findMostExpensive(Book* books, int size) {
    if (books == nullptr) {
        cerr << "Ошибка: массив не инициализирован\n";
        return;
    }
    
    if (size == 0) {
        cout << "\nБиблиотека пуста.\n";
        return;
    }
    
    int maxIndex = 0;
    for (int i = 1; i < size; i++) {
        if (books[i].price > books[maxIndex].price) {
            maxIndex = i;
        }
    }
    
    cout << "\n=== САМАЯ ДОРОГАЯ КНИГА ===\n";
    cout << "Название: " << books[maxIndex].title << "\n";
    cout << "Автор: " << books[maxIndex].author << "\n";
    cout << "Год: " << books[maxIndex].year << "\n";
    cout << "Цена: " << books[maxIndex].price << " руб.\n";
}

int main() {
    int N;
    cout << "Введите количество книг в библиотеке: ";
    
    if (!(cin >> N)) {
        cerr << "Ошибка: введено не число\n";
        return 1;
    }
    
    if (N <= 0) {
        cerr << "Ошибка: количество книг должно быть положительным\n";
        return 1;
    }
    
    if (N > 1000) {
        cerr << "Ошибка: слишком много книг (максимум 1000)\n";
        return 1;
    }
    
    Book* library = new Book[N];
    
    cout << "\nВведите данные о книгах:\n";
    for (int i = 0; i < N; i++) {
        cout << "\n--- Книга " << (i + 1) << " ---\n";
        
        cout << "Название: ";
        if (!getline(cin, library[i].title)) {
            cerr << "Ошибка чтения названия\n";
            delete[] library;
            return 1;
        }
        
        if (library[i].title.empty()) {
            cerr << "Ошибка: название не может быть пустым\n";
            delete[] library;
            return 1;
        }
        
        cout << "Автор: ";
        if (!getline(cin, library[i].author)) {
            cerr << "Ошибка чтения автора\n";
            delete[] library;
            return 1;
        }
        
        if (library[i].author.empty()) {
            cerr << "Ошибка: автор не может быть пустым\n";
            delete[] library;
            return 1;
        }
        
        cout << "Год издания: ";
        if (!(cin >> library[i].year)) {
            cerr << "Ошибка: введено не число\n";
            delete[] library;
            return 1;
        }
        
        if (library[i].year < 1000 || library[i].year > 2025) {
            cerr << "Ошибка: некорректный год (1000-2025)\n";
            delete[] library;
            return 1;
        }
        
        cout << "Цена (руб.): ";
        if (!(cin >> library[i].price)) {
            cerr << "Ошибка: введено не число\n";
            delete[] library;
            return 1;
        }
        
        if (library[i].price < 0) {
            cerr << "Ошибка: цена не может быть отрицательной\n";
            delete[] library;
            return 1;
        }
        
        if (library[i].price > 1000000) {
            cerr << "Предупреждение: очень высокая цена\n";
        }
        
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    
    printBooks(library, N);
    findMostExpensive(library, N);
    
    delete[] library;
    
    return 0;
}
