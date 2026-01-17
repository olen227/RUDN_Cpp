// DatabaseManager.cpp - реализация класса управления БД
#include "DatabaseManager.h"    // Подключение заголовочного файла
#include <iostream>              // Подключение библиотеки ввода-вывода

DatabaseManager::DatabaseManager() : db(nullptr) {}  // Конструктор, инициализация указателя нулем

DatabaseManager::~DatabaseManager() {  // Деструктор
    close();                     // Вызов метода закрытия БД
}                                // Конец деструктора

bool DatabaseManager::initialize(const std::string& path) {  // Метод инициализации БД
    dbPath = path;               // Сохранение пути к БД
    int rc = sqlite3_open(path.c_str(), &db);  // Открытие файла базы данных
    if (rc != SQLITE_OK) {       // Если произошла ошибка открытия
        std::cerr << "Ошибка открытия БД: " << sqlite3_errmsg(db) << std::endl;  // Вывод сообщения об ошибке
        return false;            // Возврат неудачи
    }                            // Конец условия
    createTables();              // Создание таблиц
    optimizeDatabase();          // Оптимизация настроек БД
    return true;                 // Возврат успеха
}                                // Конец метода initialize

void DatabaseManager::createTables() {  // Метод создания таблиц
    const char* createStudentsTable =  // SQL запрос создания таблицы студентов
        "CREATE TABLE IF NOT EXISTS students ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "email TEXT UNIQUE NOT NULL,"
        "group_name TEXT NOT NULL"
        ");";                    // Конец SQL запроса
    
    const char* createGradesTable =  // SQL запрос создания таблицы оценок
        "CREATE TABLE IF NOT EXISTS grades ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "student_id INTEGER NOT NULL,"
        "subject TEXT NOT NULL,"
        "grade INTEGER NOT NULL CHECK(grade >= 0 AND grade <= 100),"
        "FOREIGN KEY(student_id) REFERENCES students(id) ON DELETE CASCADE"
        ");";                    // Конец SQL запроса
    
    char* errMsg = nullptr;      // Указатель для сообщения об ошибке
    int rc = sqlite3_exec(db, createStudentsTable, nullptr, nullptr, &errMsg);  // Выполнение создания таблицы students
    if (rc != SQLITE_OK) {       // Проверка ошибки
        std::cerr << "Ошибка создания таблицы students: " << errMsg << std::endl;  // Вывод ошибки
        sqlite3_free(errMsg);    // Освобождение памяти сообщения
        return;                  // Выход из метода
    }                            // Конец условия
    
    rc = sqlite3_exec(db, createGradesTable, nullptr, nullptr, &errMsg);  // Выполнение создания таблицы grades
    if (rc != SQLITE_OK) {       // Проверка ошибки
        std::cerr << "Ошибка создания таблицы grades: " << errMsg << std::endl;  // Вывод ошибки
        sqlite3_free(errMsg);    // Освобождение памяти
    }                            // Конец условия
    
    const char* createIndexes =  // SQL запрос создания индексов
        "CREATE INDEX IF NOT EXISTS idx_email ON students(email);"
        "CREATE INDEX IF NOT EXISTS idx_group_name ON students(group_name);"
        "CREATE INDEX IF NOT EXISTS idx_subject ON grades(subject);"
        "CREATE INDEX IF NOT EXISTS idx_student_id ON grades(student_id);";  // Конец SQL запроса
    
    rc = sqlite3_exec(db, createIndexes, nullptr, nullptr, &errMsg);  // Создание индексов
    if (rc != SQLITE_OK) {       // Проверка ошибки
        std::cerr << "Ошибка создания индексов: " << errMsg << std::endl;  // Вывод ошибки
        sqlite3_free(errMsg);    // Освобождение памяти
    }                            // Конец условия
}                                // Конец метода createTables

void DatabaseManager::optimizeDatabase() {  // Метод оптимизации БД
    const char* pragmas =        // SQL команды оптимизации
        "PRAGMA foreign_keys = ON;"
        "PRAGMA journal_mode = WAL;"
        "PRAGMA synchronous = NORMAL;"
        "PRAGMA cache_size = 10000;"
        "PRAGMA temp_store = MEMORY;";  // Конец команд
    
    char* errMsg = nullptr;      // Указатель для ошибки
    int rc = sqlite3_exec(db, pragmas, nullptr, nullptr, &errMsg);  // Выполнение команд оптимизации
    if (rc != SQLITE_OK) {       // Проверка ошибки
        std::cerr << "Ошибка оптимизации БД: " << errMsg << std::endl;  // Вывод ошибки
        sqlite3_free(errMsg);    // Освобождение памяти
    }                            // Конец условия
}                                // Конец метода optimizeDatabase

void DatabaseManager::close() {  // Метод закрытия БД
    if (db) {                    // Если БД открыта
        sqlite3_close(db);       // Закрытие соединения с БД
        db = nullptr;            // Обнуление указателя
    }                            // Конец условия
}                                // Конец метода close

sqlite3* DatabaseManager::getHandle() const {  // Метод получения указателя на БД
    return db;                   // Возврат указателя
}                                // Конец метода

bool DatabaseManager::isOpen() const {  // Метод проверки открытия БД
    return db != nullptr;        // Возврат результата проверки
}                                // Конец метода

std::string DatabaseManager::getLastError() const {  // Метод получения последней ошибки
    if (db) {                    // Если БД существует
        return sqlite3_errmsg(db);  // Возврат сообщения об ошибке
    }                            // Конец условия
    return "Database not initialized";  // Возврат сообщения о неинициализированной БД
}                                // Конец метода
