#include <iostream>
#include <string>
#include <vector>
using namespace std;

class Book {
private:
    string title;
    string author;
    string isbn;
    int year;
    bool isAvailable;

public:
    // TODO: Создать конструктор, геттеры, сеттеры
    
    // Конструктор класса Book с инициализацией всех полей
    Book(string t, string a, string i, int y) 
        : title(t), author(a), isbn(i), year(y), isAvailable(true) {}  // По умолчанию книга доступна
    
    // Геттеры для получения информации о книге
    string getTitle() const {
        return title;
    }

    string getAuthor() const {
        return author;
    }

    string getIsbn() const {
        return isbn;
    }

    int getYear() const {
        return year;
    }

    bool getIsAvailable() const {
        return isAvailable;
    }
    
    // Сеттеры для изменения информации о книге
    void setTitle(string t) {
        title = t;
    }

    void setAuthor(string a) {
        author = a;
    }

    void setYear(int y) {
        year = y;
    }
    
    // TODO: Методы borrow() и returnBook() для изменения статуса доступности
    
    // Метод для выдачи книги читателю
    bool borrow() {
        if (isAvailable) {  // Проверка доступности книги
            isAvailable = false;  // Изменение статуса на "выдана"
            cout << "Книга \"" << title << "\" выдана." << endl;
            return true;
        }
        cout << "Книга \"" << title << "\" недоступна." << endl;
        return false;
    }
    
    // Метод для возврата книги в библиотеку
    void returnBook() {
        isAvailable = true;  // Изменение статуса на "доступна"
        cout << "Книга \"" << title << "\" возвращена." << endl;
    }
    
    // TODO: Метод displayInfo() для вывода информации о книге
    
    // Метод для вывода полной информации о книге
    void displayInfo() const {
        cout << "\"" << title << "\" - " << author << " (" << year << ")" << endl;
        cout << "ISBN: " << isbn << endl;
        cout << "Статус: " << (isAvailable ? "Доступна" : "Выдана") << endl;
    }
};

class Library {
private:
    string name;
    string address;
    vector<Book> books;

public:
    Library(string n, string addr) : name(n), address(addr) {}
    
    // TODO: Добавить методы:
    // - addBook(const Book& book)
    
    // Метод для добавления книги в библиотеку
    void addBook(const Book& book) {
        books.push_back(book);  // Добавление книги в вектор
        cout << "Книга \"" << book.getTitle() << "\" добавлена в библиотеку." << endl;
    }
    
    // - removeBook(const string& isbn)
    
    // Метод для удаления книги из библиотеки по ISBN
    bool removeBook(const string& isbn) {
        // Поиск книги с указанным ISBN
        for (auto it = books.begin(); it != books.end(); ++it) {
            if (it->getIsbn() == isbn) {
                cout << "Книга \"" << it->getTitle() << "\" удалена из библиотеки." << endl;
                books.erase(it);  // Удаление книги из вектора
                return true;
            }
        }
        cout << "Книга с ISBN " << isbn << " не найдена." << endl;
        return false;
    }
    
    // - findBook(const string& title) const
    
    // Метод для поиска книги по названию (частичное совпадение)
    void findBook(const string& title) const {
        bool found = false;
        // Перебор всех книг в библиотеке
        for (const auto& book : books) {
            // Поиск подстроки в названии книги
            if (book.getTitle().find(title) != string::npos) {
                book.displayInfo();  // Вывод информации о найденной книге
                cout << endl;
                found = true;
            }
        }
        if (!found) {
            cout << "Книги с названием \"" << title << "\" не найдены." << endl;
        }
    }
    
    // - borrowBook(const string& isbn)
    
    // Метод для выдачи книги читателю по ISBN
    bool borrowBook(const string& isbn) {
        // Поиск книги с указанным ISBN
        for (auto& book : books) {
            if (book.getIsbn() == isbn) {
                return book.borrow();  // Вызов метода borrow() у найденной книги
            }
        }
        cout << "Книга с ISBN " << isbn << " не найдена." << endl;
        return false;
    }
    
    // - returnBook(const string& isbn)
    
    // Метод для возврата книги в библиотеку по ISBN
    bool returnBook(const string& isbn) {
        // Поиск книги с указанным ISBN
        for (auto& book : books) {
            if (book.getIsbn() == isbn) {
                book.returnBook();  // Вызов метода returnBook() у найденной книги
                return true;
            }
        }
        cout << "Книга с ISBN " << isbn << " не найдена." << endl;
        return false;
    }
    
    // - displayAllBooks() const
    
    // Метод для вывода информации обо всех книгах в библиотеке
    void displayAllBooks() const {
        cout << "=== Библиотека: " << name << " ===" << endl;
        cout << "Адрес: " << address << endl;
        cout << "Всего книг: " << books.size() << endl << endl;
        
        // Вывод информации о каждой книге
        for (auto& book : books) {
            book.displayInfo();
            cout << endl;
        }
    }
    
    // - displayAvailableBooks() const
    
    // Метод для вывода информации только о доступных книгах
    void displayAvailableBooks() const {
        cout << "=== Доступные книги ===" << endl;
        int count = 0;
        // Перебор всех книг и вывод только доступных
        for (auto& book : books) {
            if (book.getIsAvailable()) {  // Проверка доступности книги
                book.displayInfo();
                cout << endl;
                count++;
            }
        }
        cout << "Всего доступно: " << count << " книг(и)" << endl;
    }
};

int main() {
    // TODO: Создать библиотеку
    // Добавить несколько книг
    // Продемонстрировать работу всех методов
    
    // Создание объекта библиотеки
    Library library("Центральная библиотека", "ул. Пушкина, д. 10");
    
    cout << "=== Добавление книг ===" << endl;
    // Добавление книг в библиотеку
    library.addBook(Book("Война и мир", "Л.Н. Толстой", "978-5-17-098342-5", 1869));
    library.addBook(Book("Преступление и наказание", "Ф.М. Достоевский", "978-5-17-098343-2", 1866));
    library.addBook(Book("Мастер и Маргарита", "М.А. Булгаков", "978-5-17-098344-9", 1967));
    library.addBook(Book("Анна Каренина", "Л.Н. Толстой", "978-5-17-098345-6", 1877));
    cout << endl;
    
    // Вывод всех книг в библиотеке
    library.displayAllBooks();
    
    cout << "=== Поиск книги ===" << endl;
    // Поиск книг по названию
    library.findBook("Толстой");
    cout << endl;
    
    cout << "=== Выдача книг ===" << endl;
    // Выдача книг читателям
    library.borrowBook("978-5-17-098342-5");
    library.borrowBook("978-5-17-098344-9");
    cout << endl;
    
    // Вывод только доступных книг
    library.displayAvailableBooks();
    cout << endl;
    
    cout << "=== Возврат книги ===" << endl;
    // Возврат книги в библиотеку
    library.returnBook("978-5-17-098342-5");
    cout << endl;
    
    // Повторный вывод доступных книг после возврата
    library.displayAvailableBooks();

}