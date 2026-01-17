#ifndef PARSER_HPP
#define PARSER_HPP

#include "JsonValue.hpp"
#include "Lexer.hpp"
#include <string>
#include <vector>
#include <functional>

namespace json {

// Исключение парсера
class ParserException : public std::runtime_error {
public:
    size_t line;
    size_t column;

    ParserException(const std::string& message, size_t l, size_t c)
        : std::runtime_error(message + " (строка " + std::to_string(l) +
                            ", столбец " + std::to_string(c) + ")"),
          line(l), column(c) {}
};

// Колбэк для отчета о прогрессе (текущая позиция, общий размер)
using ProgressCallback = std::function<void(size_t, size_t)>;

// Класс парсера JSON методом рекурсивного спуска
class Parser {
private:
    std::vector<Token> m_tokens;
    size_t m_current;
    ProgressCallback m_progressCallback;
    size_t m_totalTokens;

    // Получить текущий токен
    const Token& current() const;

    // Получить токен со смещением
    const Token& peek(size_t offset = 0) const;

    // Проверить тип текущего токена
    bool check(TokenType type) const;

    // Продвинуться к следующему токену
    void advance();

    // Проверить и продвинуться (бросает исключение при несовпадении)
    void expect(TokenType type, const std::string& message);

    // Проверить, достигнут ли конец
    bool isAtEnd() const;

    // Рекурсивные функции разбора
    JsonValue parseValue();
    JsonValue parseObject();
    JsonValue parseArray();
    JsonValue parseString();
    JsonValue parseNumber();
    JsonValue parseBool();
    JsonValue parseNull();

    // Уведомить о прогрессе
    void notifyProgress();

public:
    explicit Parser(const std::vector<Token>& tokens);
    explicit Parser(std::vector<Token>&& tokens);

    // Установить колбэк для прогресса
    void setProgressCallback(ProgressCallback callback);

    // Основной метод парсинга
    JsonValue parse();

    // Статический метод для парсинга строки
    static JsonValue parseString(const std::string& jsonStr);

    // Статический метод для парсинга файла
    static JsonValue parseFile(const std::string& filename);

    // Статический метод для парсинга файла с прогресс-баром
    static JsonValue parseFileWithProgress(const std::string& filename, ProgressCallback callback = nullptr);
};

} // namespace json

#endif // PARSER_HPP
