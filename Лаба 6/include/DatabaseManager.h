// DatabaseManager.h - класс для управления подключением к базе данных
#ifndef DATABASE_MANAGER_H      // Проверка, не определен ли макрос DATABASE_MANAGER_H
#define DATABASE_MANAGER_H      // Определение макроса для защиты от двойного включения

#include <string>                // Подключение библиотеки для работы со строками
#include <sqlite3.h>             // Подключение библиотеки SQLite3

class DatabaseManager {          // Объявление класса управления базой данных
private:                         // Закрытые члены класса
    sqlite3* db;                 // Указатель на объект базы данных SQLite
    std::string dbPath;          // Путь к файлу базы данных

public:                          // Открытые члены класса
    DatabaseManager();           // Конструктор по умолчанию
    ~DatabaseManager();          // Деструктор для закрытия БД
    
    bool initialize(const std::string& path);  // Метод инициализации БД
    void createTables();         // Метод создания таблиц
    void optimizeDatabase();     // Метод оптимизации настроек БД
    void close();                // Метод закрытия соединения
    
    sqlite3* getHandle() const;  // Получение указателя на БД
    bool isOpen() const;         // Проверка открытия БД
    std::string getLastError() const;  // Получение последней ошибки
};                               // Конец объявления класса

#endif                           // Конец условной компиляции
