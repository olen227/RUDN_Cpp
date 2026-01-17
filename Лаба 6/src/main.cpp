// main.cpp - главный модуль программы для работы с БД
#include <iostream>              // Подключение библиотеки ввода-вывода
#include <chrono>                // Подключение библиотеки для работы со временем
#include "DatabaseManager.h"    // Подключение менеджера БД
#include "StudentRepository.h"  // Подключение репозитория студентов
#include "InputValidator.h"     // Подключение валидатора

void demonstrateCRUD(StudentRepository& repo) {  // Функция демонстрации CRUD операций
    std::cout << "\n=== Демонстрация CRUD операций ===" << std::endl;  // Заголовок
    
    try {                        // Начало блока обработки исключений
        repo.addStudent("Иван Иванов", "ivan@university.ru", "CS-101");  // Добавление студента 1
        repo.addStudent("Петр Петров", "petr@university.ru", "CS-101");  // Добавление студента 2
        repo.addStudent("Мария Сидорова", "maria@university.ru", "CS-102");  // Добавление студента 3
        std::cout << "✓ Добавлено 3 студента" << std::endl;  // Сообщение об успехе
    } catch (const std::exception& e) {  // Перехват исключений
        std::cerr << "Ошибка: " << e.what() << std::endl;  // Вывод ошибки
    }                            // Конец блока try-catch
    
    auto student = repo.getStudent(1);  // Получение студента с ID=1
    if (student) {               // Если студент найден
        std::cout << "✓ Найден студент: " << student->name << " (" << student->email << ")" << std::endl;  // Вывод данных
    }                            // Конец условия
    
    repo.updateStudent(1, "Иван Иванович Иванов", "ivan.new@university.ru", "CS-101");  // Обновление данных студента
    std::cout << "✓ Обновлены данные студента ID=1" << std::endl;  // Сообщение
    
    auto allStudents = repo.getAllStudents();  // Получение всех студентов
    std::cout << "✓ Всего студентов в БД: " << allStudents.size() << std::endl;  // Вывод количества
}                                // Конец функции demonstrateCRUD

void demonstrateGrades(StudentRepository& repo) {  // Функция демонстрации работы с оценками
    std::cout << "\n=== Демонстрация работы с оценками ===" << std::endl;  // Заголовок
    
    repo.addGrade(1, "Математика", 85);  // Добавление оценки студенту 1
    repo.addGrade(1, "Физика", 90);  // Добавление еще одной оценки
    repo.addGrade(2, "Математика", 78);  // Оценка студенту 2
    repo.addGrade(2, "Физика", 82);  // Еще оценка студенту 2
    repo.addGrade(3, "Математика", 95);  // Оценка студенту 3
    std::cout << "✓ Добавлено 5 оценок" << std::endl;  // Сообщение
    
    double avgMath = repo.getAverageGradeBySubject("Математика");  // Средняя по математике
    std::cout << "✓ Средняя оценка по Математике: " << avgMath << std::endl;  // Вывод
    
    auto topStudents = repo.getTopStudents(3);  // Получение топ-3 студентов
    std::cout << "✓ Топ-3 студента:" << std::endl;  // Заголовок списка
    for (const auto& s : topStudents) {  // Перебор топ студентов
        std::cout << "  - " << s.name << " (средний балл: " << s.avgGrade << ")" << std::endl;  // Вывод информации
    }                            // Конец цикла
}                                // Конец функции demonstrateGrades

void demonstrateTransactions(StudentRepository& repo) {  // Функция демонстрации транзакций
    std::cout << "\n=== Демонстрация транзакций ===" << std::endl;  // Заголовок
    
    std::vector<Grade> grades = {{"Программирование", 88}, {"Алгоритмы", 92}};  // Вектор оценок
    bool success = repo.addStudentWithGrades("Анна Козлова", "anna@university.ru", "CS-102", grades);  // Добавление с оценками
    if (success) {               // Если операция успешна
        std::cout << "✓ Студент добавлен с оценками в одной транзакции" << std::endl;  // Сообщение
    }                            // Конец условия
}                                // Конец функции demonstrateTransactions

void demonstrateBatchInsert(StudentRepository& repo) {  // Функция демонстрации пакетной вставки
    std::cout << "\n=== Демонстрация пакетной вставки ===" << std::endl;  // Заголовок
    
    std::vector<Student> students;  // Вектор студентов
    for (int i = 0; i < 100; ++i) {  // Создание 100 студентов
        Student s;               // Создание структуры студента
        s.name = "Студент_" + std::to_string(i);  // Генерация имени
        s.email = "student" + std::to_string(i) + "@university.ru";  // Генерация email
        s.groupName = (i % 2 == 0) ? "CS-101" : "CS-102";  // Распределение по группам
        students.push_back(s);   // Добавление в вектор
    }                            // Конец цикла
    
    auto start = std::chrono::steady_clock::now();  // Начало замера времени
    bool success = repo.batchInsertStudents(students);  // Пакетная вставка
    auto end = std::chrono::steady_clock::now();  // Конец замера
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);  // Вычисление длительности
    
    if (success) {               // Если успешно
        std::cout << "✓ Добавлено 100 студентов пакетной вставкой" << std::endl;  // Сообщение
        std::cout << "✓ Время выполнения: " << duration.count() << " мс" << std::endl;  // Вывод времени
    }                            // Конец условия
}                                // Конец функции demonstrateBatchInsert

void demonstrateGroupQuery(StudentRepository& repo) {  // Функция демонстрации запросов по группе
    std::cout << "\n=== Студенты группы CS-101 ===" << std::endl;  // Заголовок
    auto students = repo.getStudentsByGroup("CS-101");  // Получение студентов группы
    std::cout << "Найдено студентов: " << students.size() << std::endl;  // Вывод количества
    for (size_t i = 0; i < std::min(size_t(5), students.size()); ++i) {  // Вывод первых 5
        std::cout << "  - " << students[i].name << " (" << students[i].email << ")" << std::endl;  // Вывод данных
    }                            // Конец цикла
}                                // Конец функции demonstrateGroupQuery

int main() {                     // Главная функция программы
    std::cout << "=== Лабораторная работа №6: Работа с базами данных ===" << std::endl;  // Заголовок программы
    
    DatabaseManager dbManager;   // Создание менеджера БД
    if (!dbManager.initialize("university.db")) {  // Инициализация БД
        std::cerr << "Не удалось инициализировать базу данных" << std::endl;  // Ошибка
        return 1;                // Завершение с кодом ошибки
    }                            // Конец условия
    std::cout << "✓ База данных инициализирована" << std::endl;  // Сообщение об успехе
    
    StudentRepository repo(dbManager.getHandle());  // Создание репозитория
    
    demonstrateCRUD(repo);       // Демонстрация CRUD
    demonstrateGrades(repo);     // Демонстрация оценок
    demonstrateTransactions(repo);  // Демонстрация транзакций
    demonstrateBatchInsert(repo);  // Демонстрация пакетной вставки
    demonstrateGroupQuery(repo); // Демонстрация запросов по группе
    
    std::cout << "\n=== Программа завершена успешно ===" << std::endl;  // Сообщение о завершении
    return 0;                    // Успешное завершение программы
}                                // Конец функции main
