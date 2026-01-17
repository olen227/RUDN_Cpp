#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>
#include <stdexcept>

namespace json {

// Типы токенов JSON
enum class TokenType {
    LeftBrace,      // {
    RightBrace,     // }
    LeftBracket,    // [
    RightBracket,   // ]
    Colon,          // :
    Comma,          // ,
    String,         // "..."
    Number,         // 123, 12.34, -5, 1e10
    True,           // true
    False,          // false
    Null,           // null
    EndOfFile       // конец входных данных
};

// Структура токена
struct Token {
    TokenType type;
    std::string value;
    size_t line;
    size_t column;

    Token(TokenType t, std::string v, size_t l, size_t c)
        : type(t), value(std::move(v)), line(l), column(c) {}
};

// Исключение лексера
class LexerException : public std::runtime_error {
public:
    size_t line;
    size_t column;

    LexerException(const std::string& message, size_t l, size_t c)
        : std::runtime_error(message + " (строка " + std::to_string(l) +
                            ", столбец " + std::to_string(c) + ")"),
          line(l), column(c) {}
};

// Класс лексера (токенизатора)
class Lexer {
private:
    std::string m_input;
    size_t m_pos;
    size_t m_line;
    size_t m_column;

    // Получить текущий символ
    char current() const;

    // Получить следующий символ (без продвижения позиции)
    char peek(size_t offset = 1) const;

    // Продвинуться на один символ
    void advance();

    // Проверить, достигнут ли конец
    bool isAtEnd() const;

    // Пропустить пробельные символы
    void skipWhitespace();

    // Разбор строки
    Token parseString();

    // Разбор числа
    Token parseNumber();

    // Разбор ключевых слов (true, false, null)
    Token parseKeyword();

    // Обработка escape-последовательностей в строке
    std::string processEscapeSequences(const std::string& str);

    // Разбор unicode escape (\uXXXX)
    char32_t parseUnicodeEscape();

    // Конвертация code point в UTF-8
    std::string codePointToUTF8(char32_t cp);

public:
    explicit Lexer(const std::string& input);

    // Получить следующий токен
    Token nextToken();

    // Токенизировать весь вход
    std::vector<Token> tokenize();
};

// Вспомогательная функция для получения названия типа токена
std::string tokenTypeName(TokenType type);

} // namespace json

#endif // LEXER_HPP
