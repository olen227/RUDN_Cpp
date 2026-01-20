#include "Validator.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace json {

Validator::Validator(bool stopOnFirstError)
    : m_current(0), m_stopOnFirstError(stopOnFirstError) {}

const Token& Validator::current() const {
    return m_tokens[m_current];
}

const Token& Validator::peek(size_t offset) const {
    size_t index = m_current + offset;
    if (index >= m_tokens.size()) {
        return m_tokens.back();
    }
    return m_tokens[index];
}

bool Validator::check(TokenType type) const {
    return current().type == type;
}

void Validator::advance() {
    if (!isAtEnd()) {
        m_current++;
    }
}

bool Validator::isAtEnd() const {
    return current().type == TokenType::EndOfFile;
}

std::string Validator::extractContext(size_t targetLine) {
    std::istringstream stream(m_input);
    std::string line;
    size_t currentLine = 0;

    while (std::getline(stream, line)) {
        currentLine++;
        if (currentLine == targetLine) {
            // Обрезаем длинные строки
            if (line.length() > 60) {
                return line.substr(0, 60) + "...";
            }
            return line;
        }
    }
    return "";
}

void Validator::addError(const std::string& message) {
    addError(message, current().line, current().column);
}

void Validator::addError(const std::string& message, size_t line, size_t col) {
    m_result.isValid = false;
    std::string context = extractContext(line);
    m_result.errors.emplace_back(line, col, message, context);
}

void Validator::synchronize() {
    // Пропускаем токены до следующей "точки синхронизации"
    while (!isAtEnd()) {
        // Останавливаемся на закрывающих скобках или запятых
        if (check(TokenType::RightBrace) ||
            check(TokenType::RightBracket) ||
            check(TokenType::Comma)) {
            return;
        }

        // Или если видим начало нового значения после ошибки
        TokenType t = current().type;
        if (t == TokenType::LeftBrace ||
            t == TokenType::LeftBracket ||
            t == TokenType::String ||
            t == TokenType::Number ||
            t == TokenType::True ||
            t == TokenType::False ||
            t == TokenType::Null) {
            return;
        }

        advance();
    }
}

bool Validator::validateValue() {
    if (!m_result.isValid && m_stopOnFirstError) {
        return false;
    }

    switch (current().type) {
        case TokenType::LeftBrace:
            return validateObject();

        case TokenType::LeftBracket:
            return validateArray();

        case TokenType::String:
        case TokenType::Number:
        case TokenType::True:
        case TokenType::False:
        case TokenType::Null:
            advance();
            return true;

        case TokenType::RightBrace:
            addError("Неожиданная закрывающая скобка '}'");
            return false;

        case TokenType::RightBracket:
            addError("Неожиданная закрывающая скобка ']'");
            return false;

        case TokenType::Comma:
            addError("Неожиданная запятая");
            return false;

        case TokenType::Colon:
            addError("Неожиданное двоеточие");
            return false;

        case TokenType::EndOfFile:
            addError("Неожиданный конец файла");
            return false;

        default:
            addError("Неожиданный токен: " + current().value);
            return false;
    }
}

bool Validator::validateObject() {
    if (!check(TokenType::LeftBrace)) {
        addError("Ожидалась '{'");
        return false;
    }
    advance();

    // Пустой объект
    if (check(TokenType::RightBrace)) {
        advance();
        return true;
    }

    bool hadError = false;

    while (true) {
        // Ключ должен быть строкой
        if (!check(TokenType::String)) {
            addError("Ожидался ключ (строка) в объекте, получено: " +
                    tokenTypeName(current().type));
            hadError = true;

            if (m_stopOnFirstError) return false;
            synchronize();

            if (check(TokenType::RightBrace)) {
                advance();
                return !hadError;
            }
            if (check(TokenType::Comma)) {
                advance();
                continue;
            }
            continue;
        }
        advance();

        // Двоеточие
        if (!check(TokenType::Colon)) {
            addError("Ожидалось ':' после ключа");
            hadError = true;

            if (m_stopOnFirstError) return false;
            // Пробуем продолжить без двоеточия
            if (!check(TokenType::String) && !check(TokenType::Number) &&
                !check(TokenType::LeftBrace) && !check(TokenType::LeftBracket) &&
                !check(TokenType::True) && !check(TokenType::False) &&
                !check(TokenType::Null)) {
                synchronize();
                continue;
            }
        } else {
            advance();
        }

        // Значение
        if (!validateValue()) {
            hadError = true;
            if (m_stopOnFirstError) return false;
            synchronize();
        }

        // Запятая или конец объекта
        if (check(TokenType::Comma)) {
            advance();

            // Проверка на trailing comma
            if (check(TokenType::RightBrace)) {
                addError("Запятая перед закрывающей скобкой '}' не допускается");
                hadError = true;
                if (m_stopOnFirstError) return false;
                advance();
                return !hadError;
            }
        } else if (check(TokenType::RightBrace)) {
            advance();
            return !hadError;
        } else {
            addError("Ожидалась ',' или '}' в объекте");
            hadError = true;

            if (m_stopOnFirstError) return false;
            synchronize();

            if (check(TokenType::RightBrace)) {
                advance();
                return !hadError;
            }
            if (isAtEnd()) {
                addError("Незакрытый объект (пропущена '}')");
                return false;
            }
        }
    }

    return !hadError;
}

bool Validator::validateArray() {
    if (!check(TokenType::LeftBracket)) {
        addError("Ожидалась '['");
        return false;
    }
    advance();

    // Пустой массив
    if (check(TokenType::RightBracket)) {
        advance();
        return true;
    }

    bool hadError = false;

    while (true) {
        // Элемент
        if (!validateValue()) {
            hadError = true;
            if (m_stopOnFirstError) return false;
            synchronize();
        }

        // Запятая или конец массива
        if (check(TokenType::Comma)) {
            advance();

            // Проверка на trailing comma
            if (check(TokenType::RightBracket)) {
                addError("Запятая перед закрывающей скобкой ']' не допускается");
                hadError = true;
                if (m_stopOnFirstError) return false;
                advance();
                return !hadError;
            }
        } else if (check(TokenType::RightBracket)) {
            advance();
            return !hadError;
        } else {
            addError("Ожидалась ',' или ']' в массиве");
            hadError = true;

            if (m_stopOnFirstError) return false;
            synchronize();

            if (check(TokenType::RightBracket)) {
                advance();
                return !hadError;
            }
            if (isAtEnd()) {
                addError("Незакрытый массив (пропущена ']')");
                return false;
            }
        }
    }

    return !hadError;
}

ValidationResult Validator::validate(const std::string& jsonStr) {
    m_input = jsonStr;
    m_current = 0;
    m_result = ValidationResult();

    // Подсчёт строк
    m_result.lineCount = std::count(jsonStr.begin(), jsonStr.end(), '\n') + 1;

    // Токенизация
    try {
        Lexer lexer(jsonStr);
        m_tokens = lexer.tokenize();
        m_result.tokenCount = m_tokens.size();
    } catch (const LexerException& e) {
        m_result.isValid = false;
        m_result.errors.emplace_back(e.line, e.column, e.what(), extractContext(e.line));
        return m_result;
    }

    if (m_tokens.empty() || (m_tokens.size() == 1 && m_tokens[0].type == TokenType::EndOfFile)) {
        m_result.isValid = false;
        m_result.errors.emplace_back(1, 1, "Пустой JSON", "");
        return m_result;
    }

    // Валидация
    validateValue();

    // Проверка на лишние данные после JSON
    if (!isAtEnd()) {
        addError("Неожиданные данные после JSON");
    }

    return m_result;
}

ValidationResult Validator::validateFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        ValidationResult result;
        result.isValid = false;
        result.errors.emplace_back(0, 0, "Не удалось открыть файл: " + filename, "");
        return result;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return validate(buffer.str());
}

bool Validator::isValid(const std::string& jsonStr) {
    Validator validator(true);
    return validator.validate(jsonStr).isValid;
}

ValidationResult Validator::check(const std::string& jsonStr) {
    Validator validator(false);
    return validator.validate(jsonStr);
}

} // namespace json
