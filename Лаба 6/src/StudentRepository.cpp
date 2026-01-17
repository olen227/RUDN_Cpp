// StudentRepository.cpp - реализация репозитория студентов (часть 1)
#include "StudentRepository.h"  // Подключение заголовочного файла
#include "InputValidator.h"     // Подключение валидатора
#include <iostream>              // Подключение библиотеки ввода-вывода
#include <stdexcept>             // Подключение библиотеки исключений

StudentRepository::StudentRepository(sqlite3* database) : db(database) {}  // Конструктор с инициализацией БД

bool StudentRepository::addStudent(const std::string& name, const std::string& email, const std::string& group) {  // Метод добавления студента
    if (!InputValidator::isValidNameLength(name)) {  // Проверка длины имени
        throw std::invalid_argument("Имя должно быть от 2 до 100 символов");  // Исключение при ошибке
    }                            // Конец проверки
    if (!InputValidator::isValidEmail(email)) {  // Проверка формата email
        throw std::invalid_argument("Неверный формат email");  // Исключение при ошибке
    }                            // Конец проверки
    if (!InputValidator::isValidGroupName(group)) {  // Проверка формата группы
        throw std::invalid_argument("Неверный формат группы (требуется XX-YYY)");  // Исключение при ошибке
    }                            // Конец проверки
    if (!InputValidator::isSafeInput(name)) {  // Проверка безопасности имени
        throw std::invalid_argument("Обнаружена потенциальная SQL-инъекция в имени");  // Исключение при ошибке
    }                            // Конец проверки
    
    const char* sql = "INSERT INTO students (name, email, group_name) VALUES (?, ?, ?);";  // SQL запрос с параметрами
    sqlite3_stmt* stmt;          // Указатель на подготовленное выражение
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);  // Подготовка SQL выражения
    if (rc != SQLITE_OK) {       // Проверка ошибки подготовки
        std::cerr << "Ошибка подготовки запроса: " << sqlite3_errmsg(db) << std::endl;  // Вывод ошибки
        return false;            // Возврат неудачи
    }                            // Конец условия
    
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);  // Привязка параметра 1 (имя)
    sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_TRANSIENT);  // Привязка параметра 2 (email)
    sqlite3_bind_text(stmt, 3, group.c_str(), -1, SQLITE_TRANSIENT);  // Привязка параметра 3 (группа)
    
    rc = sqlite3_step(stmt);     // Выполнение запроса
    sqlite3_finalize(stmt);      // Освобождение ресурсов выражения
    
    if (rc != SQLITE_DONE) {     // Проверка успешности выполнения
        std::cerr << "Ошибка добавления студента: " << sqlite3_errmsg(db) << std::endl;  // Вывод ошибки
        return false;            // Возврат неудачи
    }                            // Конец условия
    return true;                 // Возврат успеха
}                                // Конец метода addStudent

std::optional<Student> StudentRepository::getStudent(int id) {  // Метод получения студента по ID
    const char* sql = "SELECT id, name, email, group_name FROM students WHERE id = ?;";  // SQL запрос
    sqlite3_stmt* stmt;          // Указатель на выражение
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);  // Подготовка запроса
    if (rc != SQLITE_OK) {       // Проверка ошибки
        return std::nullopt;     // Возврат пустого optional
    }                            // Конец условия
    
    sqlite3_bind_int(stmt, 1, id);  // Привязка ID студента
    rc = sqlite3_step(stmt);     // Выполнение запроса
    
    if (rc == SQLITE_ROW) {      // Если найдена строка
        Student student;         // Создание структуры студента
        student.id = sqlite3_column_int(stmt, 0);  // Получение ID из результата
        student.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));  // Получение имени
        student.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));  // Получение email
        student.groupName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));  // Получение группы
        sqlite3_finalize(stmt);  // Освобождение ресурсов
        return student;          // Возврат найденного студента
    }                            // Конец условия
    
    sqlite3_finalize(stmt);      // Освобождение ресурсов
    return std::nullopt;         // Возврат пустого optional (студент не найден)
}                                // Конец метода getStudent

bool StudentRepository::updateStudent(int id, const std::string& name, const std::string& email, const std::string& group) {  // Метод обновления студента
    if (!InputValidator::isValidNameLength(name)) {  // Проверка длины имени
        throw std::invalid_argument("Имя должно быть от 2 до 100 символов");  // Исключение
    }                            // Конец проверки
    if (!InputValidator::isValidEmail(email)) {  // Проверка email
        throw std::invalid_argument("Неверный формат email");  // Исключение
    }                            // Конец проверки
    if (!InputValidator::isValidGroupName(group)) {  // Проверка группы
        throw std::invalid_argument("Неверный формат группы");  // Исключение
    }                            // Конец проверки
    
    const char* sql = "UPDATE students SET name = ?, email = ?, group_name = ? WHERE id = ?;";  // SQL запрос обновления
    sqlite3_stmt* stmt;          // Указатель на выражение
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);  // Подготовка запроса
    if (rc != SQLITE_OK) {       // Проверка ошибки
        return false;            // Возврат неудачи
    }                            // Конец условия
    
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);  // Привязка имени
    sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_TRANSIENT);  // Привязка email
    sqlite3_bind_text(stmt, 3, group.c_str(), -1, SQLITE_TRANSIENT);  // Привязка группы
    sqlite3_bind_int(stmt, 4, id);  // Привязка ID
    
    rc = sqlite3_step(stmt);     // Выполнение запроса
    sqlite3_finalize(stmt);      // Освобождение ресурсов
    return rc == SQLITE_DONE;    // Возврат результата (успех/неудача)
}                                // Конец метода updateStudent

bool StudentRepository::deleteStudent(int id) {  // Метод удаления студента
    const char* sql = "DELETE FROM students WHERE id = ?;";  // SQL запрос удаления
    sqlite3_stmt* stmt;          // Указатель на выражение
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);  // Подготовка запроса
    if (rc != SQLITE_OK) {       // Проверка ошибки
        return false;            // Возврат неудачи
    }                            // Конец условия
    
    sqlite3_bind_int(stmt, 1, id);  // Привязка ID студента
    rc = sqlite3_step(stmt);     // Выполнение запроса
    sqlite3_finalize(stmt);      // Освобождение ресурсов
    return rc == SQLITE_DONE;    // Возврат результата
}                                // Конец метода deleteStudent

std::vector<Student> StudentRepository::getAllStudents() {  // Метод получения всех студентов
    std::vector<Student> students;  // Вектор для хранения студентов
    const char* sql = "SELECT id, name, email, group_name FROM students;";  // SQL запрос всех студентов
    sqlite3_stmt* stmt;          // Указатель на выражение
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);  // Подготовка запроса
    if (rc != SQLITE_OK) {       // Проверка ошибки
        return students;         // Возврат пустого вектора
    }                            // Конец условия
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {  // Пока есть строки в результате
        Student student;         // Создание структуры студента
        student.id = sqlite3_column_int(stmt, 0);  // Получение ID
        student.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));  // Получение имени
        student.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));  // Получение email
        student.groupName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));  // Получение группы
        students.push_back(student);  // Добавление студента в вектор
    }                            // Конец цикла
    
    sqlite3_finalize(stmt);      // Освобождение ресурсов
    return students;             // Возврат вектора студентов
}                                // Конец метода getAllStudents

bool StudentRepository::addGrade(int studentId, const std::string& subject, int grade) {  // Метод добавления оценки
    if (!InputValidator::isValidGrade(grade)) {  // Проверка диапазона оценки
        throw std::invalid_argument("Оценка должна быть в диапазоне 0-100");  // Исключение
    }                            // Конец проверки
    
    const char* sql = "INSERT INTO grades (student_id, subject, grade) VALUES (?, ?, ?);";  // SQL запрос
    sqlite3_stmt* stmt;          // Указатель на выражение
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);  // Подготовка запроса
    if (rc != SQLITE_OK) {       // Проверка ошибки
        return false;            // Возврат неудачи
    }                            // Конец условия
    
    sqlite3_bind_int(stmt, 1, studentId);  // Привязка ID студента
    sqlite3_bind_text(stmt, 2, subject.c_str(), -1, SQLITE_TRANSIENT);  // Привязка предмета
    sqlite3_bind_int(stmt, 3, grade);  // Привязка оценки
    
    rc = sqlite3_step(stmt);     // Выполнение запроса
    sqlite3_finalize(stmt);      // Освобождение ресурсов
    return rc == SQLITE_DONE;    // Возврат результата
}                                // Конец метода addGrade

bool StudentRepository::addStudentWithGrades(const std::string& name, const std::string& email, const std::string& group, const std::vector<Grade>& grades) {  // Метод добавления студента с оценками
    char* errMsg = nullptr;      // Указатель для сообщения об ошибке
    int rc = sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg);  // Начало транзакции
    if (rc != SQLITE_OK) {       // Проверка ошибки начала транзакции
        std::cerr << "Ошибка начала транзакции: " << errMsg << std::endl;  // Вывод ошибки
        sqlite3_free(errMsg);    // Освобождение памяти
        return false;            // Возврат неудачи
    }                            // Конец условия
    
    if (!addStudent(name, email, group)) {  // Попытка добавить студента
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);  // Откат транзакции при ошибке
        return false;            // Возврат неудачи
    }                            // Конец условия
    
    int studentId = sqlite3_last_insert_rowid(db);  // Получение ID только что добавленного студента
    
    for (const auto& grade : grades) {  // Перебор всех оценок
        if (!addGrade(studentId, grade.subject, grade.grade)) {  // Попытка добавить оценку
            sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);  // Откат при ошибке
            return false;        // Возврат неудачи
        }                        // Конец условия
    }                            // Конец цикла
    
    rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &errMsg);  // Фиксация транзакции
    if (rc != SQLITE_OK) {       // Проверка ошибки фиксации
        std::cerr << "Ошибка фиксации транзакции: " << errMsg << std::endl;  // Вывод ошибки
        sqlite3_free(errMsg);    // Освобождение памяти
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);  // Откат транзакции
        return false;            // Возврат неудачи
    }                            // Конец условия
    return true;                 // Возврат успеха
}                                // Конец метода addStudentWithGrades

std::vector<Student> StudentRepository::getStudentsByGroup(const std::string& groupName) {  // Метод получения студентов по группе
    std::vector<Student> students;  // Вектор для результатов
    const char* sql = "SELECT id, name, email, group_name FROM students WHERE group_name = ?;";  // SQL запрос с фильтром
    sqlite3_stmt* stmt;          // Указатель на выражение
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);  // Подготовка запроса
    if (rc != SQLITE_OK) {       // Проверка ошибки
        return students;         // Возврат пустого вектора
    }                            // Конец условия
    
    sqlite3_bind_text(stmt, 1, groupName.c_str(), -1, SQLITE_TRANSIENT);  // Привязка названия группы
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {  // Пока есть строки
        Student student;         // Создание структуры студента
        student.id = sqlite3_column_int(stmt, 0);  // Получение ID
        student.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));  // Получение имени
        student.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));  // Получение email
        student.groupName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));  // Получение группы
        students.push_back(student);  // Добавление в вектор
    }                            // Конец цикла
    
    sqlite3_finalize(stmt);      // Освобождение ресурсов
    return students;             // Возврат вектора студентов
}                                // Конец метода getStudentsByGroup

double StudentRepository::getAverageGradeBySubject(const std::string& subject) {  // Метод получения средней оценки по предмету
    const char* sql = "SELECT AVG(grade) FROM grades WHERE subject = ?;";  // SQL запрос с агрегацией
    sqlite3_stmt* stmt;          // Указатель на выражение
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);  // Подготовка запроса
    if (rc != SQLITE_OK) {       // Проверка ошибки
        return 0.0;              // Возврат 0 при ошибке
    }                            // Конец условия
    
    sqlite3_bind_text(stmt, 1, subject.c_str(), -1, SQLITE_TRANSIENT);  // Привязка названия предмета
    
    double average = 0.0;        // Переменная для средней оценки
    if (sqlite3_step(stmt) == SQLITE_ROW) {  // Если получен результат
        average = sqlite3_column_double(stmt, 0);  // Получение среднего значения
    }                            // Конец условия
    
    sqlite3_finalize(stmt);      // Освобождение ресурсов
    return average;              // Возврат средней оценки
}                                // Конец метода getAverageGradeBySubject

std::vector<StudentWithAvg> StudentRepository::getTopStudents(int limit) {  // Метод получения топ студентов
    std::vector<StudentWithAvg> topStudents;  // Вектор для результатов
    const char* sql =            // SQL запрос с JOIN и агрегацией
        "SELECT s.id, s.name, AVG(g.grade) as avg_grade "
        "FROM students s "
        "JOIN grades g ON s.id = g.student_id "
        "GROUP BY s.id "
        "ORDER BY avg_grade DESC "
        "LIMIT ?;";              // Конец SQL запроса
    
    sqlite3_stmt* stmt;          // Указатель на выражение
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);  // Подготовка запроса
    if (rc != SQLITE_OK) {       // Проверка ошибки
        return topStudents;      // Возврат пустого вектора
    }                            // Конец условия
    
    sqlite3_bind_int(stmt, 1, limit);  // Привязка лимита
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {  // Пока есть строки
        StudentWithAvg student;  // Создание структуры
        student.id = sqlite3_column_int(stmt, 0);  // Получение ID
        student.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));  // Получение имени
        student.avgGrade = sqlite3_column_double(stmt, 2);  // Получение средней оценки
        topStudents.push_back(student);  // Добавление в вектор
    }                            // Конец цикла
    
    sqlite3_finalize(stmt);      // Освобождение ресурсов
    return topStudents;          // Возврат топ студентов
}                                // Конец метода getTopStudents

bool StudentRepository::batchInsertStudents(const std::vector<Student>& students) {  // Метод пакетной вставки студентов
    char* errMsg = nullptr;      // Указатель для ошибки
    int rc = sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg);  // Начало транзакции
    if (rc != SQLITE_OK) {       // Проверка ошибки
        std::cerr << "Ошибка начала транзакции: " << errMsg << std::endl;  // Вывод ошибки
        sqlite3_free(errMsg);    // Освобождение памяти
        return false;            // Возврат неудачи
    }                            // Конец условия
    
    const char* sql = "INSERT INTO students (name, email, group_name) VALUES (?, ?, ?);";  // SQL запрос
    sqlite3_stmt* stmt;          // Указатель на выражение
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);  // Подготовка запроса
    if (rc != SQLITE_OK) {       // Проверка ошибки
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);  // Откат транзакции
        return false;            // Возврат неудачи
    }                            // Конец условия
    
    for (const auto& student : students) {  // Перебор всех студентов
        sqlite3_bind_text(stmt, 1, student.name.c_str(), -1, SQLITE_TRANSIENT);  // Привязка имени
        sqlite3_bind_text(stmt, 2, student.email.c_str(), -1, SQLITE_TRANSIENT);  // Привязка email
        sqlite3_bind_text(stmt, 3, student.groupName.c_str(), -1, SQLITE_TRANSIENT);  // Привязка группы
        
        rc = sqlite3_step(stmt); // Выполнение запроса
        if (rc != SQLITE_DONE) { // Проверка успешности
            sqlite3_finalize(stmt);  // Освобождение ресурсов
            sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);  // Откат транзакции
            return false;        // Возврат неудачи
        }                        // Конец условия
        sqlite3_reset(stmt);     // Сброс выражения для повторного использования
    }                            // Конец цикла
    
    sqlite3_finalize(stmt);      // Освобождение ресурсов
    rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &errMsg);  // Фиксация транзакции
    if (rc != SQLITE_OK) {       // Проверка ошибки
        std::cerr << "Ошибка фиксации: " << errMsg << std::endl;  // Вывод ошибки
        sqlite3_free(errMsg);    // Освобождение памяти
        return false;            // Возврат неудачи
    }                            // Конец условия
    return true;                 // Возврат успеха
}                                // Конец метода batchInsertStudents
