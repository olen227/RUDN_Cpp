// StudentRepository.h - класс для работы со студентами
#ifndef STUDENT_REPOSITORY_H    // Проверка определения макроса
#define STUDENT_REPOSITORY_H    // Определение макроса защиты

#include <string>                // Подключение библиотеки для строк
#include <vector>                // Подключение библиотеки для векторов
#include <optional>              // Подключение библиотеки для optional
#include <sqlite3.h>             // Подключение SQLite3

struct Student {                 // Структура данных студента
    int id;                      // Уникальный идентификатор
    std::string name;            // ФИО студента
    std::string email;           // Email студента
    std::string groupName;       // Название группы
};                               // Конец структуры

struct Grade {                   // Структура оценки
    std::string subject;         // Название предмета
    int grade;                   // Оценка (0-100)
};                               // Конец структуры

struct StudentWithAvg {          // Структура студента со средней оценкой
    int id;                      // ID студента
    std::string name;            // ФИО студента
    double avgGrade;             // Средняя оценка
};                               // Конец структуры

class StudentRepository {        // Класс репозитория студентов
private:                         // Закрытые члены
    sqlite3* db;                 // Указатель на базу данных

public:                          // Открытые методы
    StudentRepository(sqlite3* database);  // Конструктор с БД
    
    bool addStudent(const std::string& name, const std::string& email, const std::string& group);  // Добавление студента
    std::optional<Student> getStudent(int id);  // Получение студента по ID
    bool updateStudent(int id, const std::string& name, const std::string& email, const std::string& group);  // Обновление данных
    bool deleteStudent(int id);  // Удаление студента
    std::vector<Student> getAllStudents();  // Получение всех студентов
    
    bool addGrade(int studentId, const std::string& subject, int grade);  // Добавление оценки
    bool addStudentWithGrades(const std::string& name, const std::string& email, const std::string& group, const std::vector<Grade>& grades);  // Добавление с оценками
    
    std::vector<Student> getStudentsByGroup(const std::string& groupName);  // Студенты по группе
    double getAverageGradeBySubject(const std::string& subject);  // Средняя оценка по предмету
    std::vector<StudentWithAvg> getTopStudents(int limit);  // Топ студентов
    
    bool batchInsertStudents(const std::vector<Student>& students);  // Пакетная вставка
};                               // Конец класса

#endif                           // Конец условной компиляции
