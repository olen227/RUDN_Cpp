// InputValidator.h - класс для валидации входных данных
#ifndef INPUT_VALIDATOR_H       // Проверка определения макроса
#define INPUT_VALIDATOR_H       // Определение макроса защиты

#include <string>                // Подключение библиотеки для строк
#include <regex>                 // Подключение библиотеки для регулярных выражений

class InputValidator {           // Класс валидатора входных данных
public:                          // Открытые методы
    static bool isValidEmail(const std::string& email);  // Проверка корректности email
    static bool isSafeInput(const std::string& input);   // Проверка безопасности ввода (SQL-инъекции)
    static bool isValidNameLength(const std::string& name);  // Проверка длины имени
    static bool isValidGrade(int grade);  // Проверка диапазона оценки (0-100)
    static bool isValidGroupName(const std::string& group);  // Проверка формата названия группы
};                               // Конец класса

#endif                           // Конец условной компиляции
