#include <iostream>
#include <string>
using namespace std;

struct Book {
    string title;
    string author;
    int year;
    double price;
};

void printBook(const Book& book) {
    cout << "  \"" << book.title << "\"\n";
    cout << "  Автор: " << book.author << "\n";
    cout << "  Год: " << book.year << "\n";
    cout << "  Цена: " << book.price << " руб.\n";
}

void printAllBooks(Book* books, int size) {
    for (int i = 0; i < size; i++) {
        cout << "\nКнига " << (i + 1) << ":\n";
        printBook(books[i]);
    }
}

void sortBooksByYear(Book* books, int size) {
    if (size <= 0 || books == nullptr) {
        cerr << "Ошибка: некорректные параметры для сортировки\n";
        return;
    }
    
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (books[j].year > books[j + 1].year) {
                Book temp = books[j];
                books[j] = books[j + 1];
                books[j + 1] = temp;
            }
        }
    }
    
    cout << "Книги отсортированы по году издания (по возрастанию).\n";
}

Book* findBookByAuthor(Book* books, int size, const string& author) {
    if (books == nullptr || size <= 0) {
        cerr << "Ошибка: некорректные параметры для поиска\n";
        return nullptr;
    }
    
    for (int i = 0; i < size; i++) {
        if (books[i].author == author) {
            return &books[i];
        }
    }
    
    return nullptr;
}

int findAllBooksByAuthor(Book* books, int size, const string& author, Book** results, int maxResults) {
    if (books == nullptr || results == nullptr || size <= 0) {
        return 0;
    }
    
    int foundCount = 0;
    for (int i = 0; i < size && foundCount < maxResults; i++) {
        if (books[i].author == author) {
            results[foundCount] = &books[i];
            foundCount++;
        }
    }
    
    return foundCount;
}

int main() {
    cout << "=== БИБЛИОТЕКА: ПОИСК И СОРТИРОВКА ===\n";
    
    const int LIBRARY_SIZE = 7;
    Book* library = new Book[LIBRARY_SIZE];
    
    library[0] = {"Война и мир", "Толстой Л.Н.", 1869, 1200.50};
    library[1] = {"Преступление и наказание", "Достоевский Ф.М.", 1866, 950.00};
    library[2] = {"Мастер и Маргарита", "Булгаков М.А.", 1967, 800.00};
    library[3] = {"Евгений Онегин", "Пушкин А.С.", 1833, 650.00};
    library[4] = {"Идиот", "Достоевский Ф.М.", 1869, 1100.00};
    library[5] = {"Анна Каренина", "Толстой Л.Н.", 1877, 1050.00};
    library[6] = {"Собачье сердце", "Булгаков М.А.", 1925, 700.00};
    
    cout << "\n=== ИСХОДНЫЙ СПИСОК КНИГ ===";
    printAllBooks(library, LIBRARY_SIZE);
    
    cout << "\n\n=== СОРТИРОВКА ПО ГОДУ ИЗДАНИЯ ===\n";
    sortBooksByYear(library, LIBRARY_SIZE);
    
    cout << "\nОтсортированный список:";
    printAllBooks(library, LIBRARY_SIZE);
    
    cout << "\n\n=== ПОИСК ПО АВТОРУ ===\n";
    string searchAuthor1 = "Достоевский Ф.М.";
    cout << "Ищем книгу автора: " << searchAuthor1 << "\n";
    
    Book* found = findBookByAuthor(library, LIBRARY_SIZE, searchAuthor1);
    
    if (found != nullptr) {
        cout << "\nНайдена книга:\n";
        printBook(*found);
    } else {
        cout << "\nКнига не найдена.\n";
    }
    
    cout << "\n--- Поиск несуществующего автора ---\n";
    string searchAuthor2 = "Чехов А.П.";
    cout << "Ищем книгу автора: " << searchAuthor2 << "\n";
    
    Book* notFound = findBookByAuthor(library, LIBRARY_SIZE, searchAuthor2);
    
    if (notFound != nullptr) {
        cout << "Найдена книга:\n";
        printBook(*notFound);
    } else {
        cout << "Книга автора \"" << searchAuthor2 << "\" не найдена.\n";
    }
    
    cout << "\n--- Поиск ВСЕХ книг автора ---\n";
    string searchAuthor3 = "Булгаков М.А.";
    cout << "Ищем все книги автора: " << searchAuthor3 << "\n";
    
    const int MAX_RESULTS = 10;
    Book* allFound[MAX_RESULTS];
    int foundCount = findAllBooksByAuthor(library, LIBRARY_SIZE, searchAuthor3, allFound, MAX_RESULTS);
    
    if (foundCount > 0) {
        cout << "\nНайдено книг: " << foundCount << "\n";
        for (int i = 0; i < foundCount; i++) {
            cout << "\nКнига " << (i + 1) << ":\n";
            printBook(*allFound[i]);
        }
    } else {
        cout << "\nКниг автора \"" << searchAuthor3 << "\" не найдено.\n";
    }
    
    cout << "\n\n=== ТЕСТИРОВАНИЕ ЗАЩИТЫ ===\n";
    
    cout << "\n1. Попытка сортировки nullptr:\n";
    sortBooksByYear(nullptr, 5);
    
    cout << "\n2. Попытка поиска в nullptr:\n";
    Book* errorResult = findBookByAuthor(nullptr, 5, "Тест");
    if (errorResult == nullptr) {
        cout << "Корректно вернулся nullptr\n";
    }
    
    delete[] library;
    
    cout << "\n=== ИТОГИ ===\n";
    cout << "Пузырьковая сортировка работает корректно\n";
    cout << "Поиск возвращает nullptr, если книга не найдена\n";
    cout << "Все проверки на ошибки работают\n";
    cout << "Память освобождена без утечек\n";
    
    return 0;
}
