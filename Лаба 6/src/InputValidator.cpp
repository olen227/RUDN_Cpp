// InputValidator.cpp - реализация валидатора входных данных
#include "InputValidator.h"     // Подключение заголовочного файла
#include <algorithm>             // Подключение библиотеки алгоритмов

bool InputValidator::isValidEmail(const std::string& email) {  // Метод проверки email
    std::regex emailRegex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");  // Регулярное выражение для email
    return std::regex_match(email, emailRegex);  // Проверка соответствия регулярному выражению
}                                // Конец метода isValidEmail

bool InputValidator::isSafeInput(const std::string& input) {  // Метод проверки безопасности ввода
    std::string dangerous[] = {"--", ";", "/*", "*/", "xp_", "sp_", "DROP", "DELETE", "INSERT", "UPDATE", "SELECT", "UNION", "ALTER", "CREATE"};  // Опасные SQL-паттерны
    std::string upperInput = input;  // Копия строки для преобразования
    std::transform(upperInput.begin(), upperInput.end(), upperInput.begin(), ::toupper);  // Преобразование в верхний регистр
    
    for (const auto& pattern : dangerous) {  // Перебор опасных паттернов
        if (upperInput.find(pattern) != std::string::npos) {  // Если найден опасный паттерн
            return false;        // Возврат false (небезопасный ввод)
        }                        // Конец условия
    }                            // Конец цикла
    return true;                 // Возврат true (безопасный ввод)
}                                // Конец метода isSafeInput

bool InputValidator::isValidNameLength(const std::string& name) {  // Метод проверки длины имени
    return name.length() >= 2 && name.length() <= 100;  // Проверка диапазона длины (2-100 символов)
}                                // Конец метода isValidNameLength

bool InputValidator::isValidGrade(int grade) {  // Метод проверки диапазона оценки
    return grade >= 0 && grade <= 100;  // Проверка диапазона 0-100
}                                // Конец метода isValidGrade

bool InputValidator::isValidGroupName(const std::string& group) {  // Метод проверки названия группы
    if (group.empty() || group.length() > 20) {  // Проверка пустоты и максимальной длины
        return false;            // Возврат false если некорректно
    }                            // Конец условия
    std::regex groupRegex(R"([A-Z]{2,4}-\d{2,3})");  // Регулярное выражение для формата группы (например CS-101)
    return std::regex_match(group, groupRegex);  // Проверка соответствия формату
}                                // Конец метода isValidGroupName
