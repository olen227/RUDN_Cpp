#ifndef VALIDATOR_HPP
#define VALIDATOR_HPP

#include "Lexer.hpp"
#include <string>
#include <vector>

namespace json {

// Информация об ошибке валидации
struct ValidationError {
    size_t line;
    size_t column;
    std::string message;
    std::string context;  // Фрагмент JSON вокруг ошибки

    ValidationError(size_t l, size_t c, const std::string& msg, const std::string& ctx = "")
        : line(l), column(c), message(msg), context(ctx) {}
};

// Результат валидации
struct ValidationResult {
    bool isValid;
    std::vector<ValidationError> errors;
    size_t tokenCount;
    size_t lineCount;

    ValidationResult() : isValid(true), tokenCount(0), lineCount(0) {}
};

// Класс валидатора JSON
class Validator {
private:
    std::string m_input;
    std::vector<Token> m_tokens;
    size_t m_current;
    ValidationResult m_result;
    bool m_stopOnFirstError;

    // Получить текущий токен
    const Token& current() const;
    const Token& peek(size_t offset = 0) const;
    bool check(TokenType type) const;
    void advance();
    bool isAtEnd() const;

    // Добавить ошибку
    void addError(const std::string& message);
    void addError(const std::string& message, size_t line, size_t col);

    // Извлечь контекст (строку вокруг ошибки)
    std::string extractContext(size_t line);

    // Рекурсивные функции валидации
    bool validateValue();
    bool validateObject();
    bool validateArray();

    // Попытка восстановления после ошибки
    void synchronize();

public:
    explicit Validator(bool stopOnFirstError = false);

    // Валидация строки JSON
    ValidationResult validate(const std::string& jsonStr);

    // Валидация файла
    ValidationResult validateFile(const std::string& filename);

    // Статический метод для быстрой проверки
    static bool isValid(const std::string& jsonStr);
    static ValidationResult check(const std::string& jsonStr);
};

} // namespace json

#endif // VALIDATOR_HPP
