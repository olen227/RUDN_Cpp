#include "Parser.hpp"
#include <fstream>
#include <sstream>
#include <cstdlib>

namespace json {

Parser::Parser(const std::vector<Token>& tokens)
    : m_tokens(tokens), m_current(0), m_progressCallback(nullptr), m_totalTokens(tokens.size()) {}

Parser::Parser(std::vector<Token>&& tokens)
    : m_tokens(std::move(tokens)), m_current(0), m_progressCallback(nullptr), m_totalTokens(m_tokens.size()) {}

void Parser::setProgressCallback(ProgressCallback callback) {
    m_progressCallback = callback;
}

void Parser::notifyProgress() {
    if (m_progressCallback && m_totalTokens > 0) {
        // Вызываем колбэк каждые 1000 токенов для производительности
        if (m_current % 1000 == 0 || m_current == m_totalTokens) {
            m_progressCallback(m_current, m_totalTokens);
        }
    }
}

const Token& Parser::current() const {
    return m_tokens[m_current];
}

const Token& Parser::peek(size_t offset) const {
    size_t index = m_current + offset;
    if (index >= m_tokens.size()) {
        return m_tokens.back(); // EndOfFile
    }
    return m_tokens[index];
}

bool Parser::check(TokenType type) const {
    return current().type == type;
}

void Parser::advance() {
    if (!isAtEnd()) {
        m_current++;
        notifyProgress();
    }
}

void Parser::expect(TokenType type, const std::string& message) {
    if (!check(type)) {
        throw ParserException(message + ", получено: " + tokenTypeName(current().type),
                             current().line, current().column);
    }
    advance();
}

bool Parser::isAtEnd() const {
    return current().type == TokenType::EndOfFile;
}

JsonValue Parser::parse() {
    if (isAtEnd()) {
        throw ParserException("Пустой JSON", 1, 1);
    }

    JsonValue result = parseValue();

    if (!isAtEnd()) {
        throw ParserException("Неожиданные данные после JSON",
                             current().line, current().column);
    }

    return result;
}

JsonValue Parser::parseValue() {
    switch (current().type) {
        case TokenType::LeftBrace:
            return parseObject();
        case TokenType::LeftBracket:
            return parseArray();
        case TokenType::String:
            return parseString();
        case TokenType::Number:
            return parseNumber();
        case TokenType::True:
        case TokenType::False:
            return parseBool();
        case TokenType::Null:
            return parseNull();
        default:
            throw ParserException("Неожиданный токен: " + tokenTypeName(current().type),
                                 current().line, current().column);
    }
}

JsonValue Parser::parseObject() {
    expect(TokenType::LeftBrace, "Ожидалась '{'");

    JsonObject obj;

    // Пустой объект
    if (check(TokenType::RightBrace)) {
        advance();
        return JsonValue(std::move(obj));
    }

    while (true) {
        // Ключ (строка)
        if (!check(TokenType::String)) {
            throw ParserException("Ожидался ключ (строка) в объекте",
                                 current().line, current().column);
        }
        std::string key = current().value;
        advance();

        // Двоеточие
        expect(TokenType::Colon, "Ожидалось ':'");

        // Значение
        JsonValue value = parseValue();
        obj[key] = std::move(value);

        // Запятая или конец объекта
        if (check(TokenType::Comma)) {
            advance();
            // Проверка на trailing comma (не допускается в JSON)
            if (check(TokenType::RightBrace)) {
                throw ParserException("Запятая перед закрывающей скобкой не допускается",
                                     current().line, current().column);
            }
        } else if (check(TokenType::RightBrace)) {
            advance();
            break;
        } else {
            throw ParserException("Ожидалась ',' или '}'",
                                 current().line, current().column);
        }
    }

    return JsonValue(std::move(obj));
}

JsonValue Parser::parseArray() {
    expect(TokenType::LeftBracket, "Ожидалась '['");

    JsonArray arr;

    // Пустой массив
    if (check(TokenType::RightBracket)) {
        advance();
        return JsonValue(std::move(arr));
    }

    while (true) {
        // Элемент
        arr.push_back(parseValue());

        // Запятая или конец массива
        if (check(TokenType::Comma)) {
            advance();
            // Проверка на trailing comma
            if (check(TokenType::RightBracket)) {
                throw ParserException("Запятая перед закрывающей скобкой не допускается",
                                     current().line, current().column);
            }
        } else if (check(TokenType::RightBracket)) {
            advance();
            break;
        } else {
            throw ParserException("Ожидалась ',' или ']'",
                                 current().line, current().column);
        }
    }

    return JsonValue(std::move(arr));
}

JsonValue Parser::parseString() {
    std::string value = current().value;
    advance();
    return JsonValue(std::move(value));
}

JsonValue Parser::parseNumber() {
    double value = std::stod(current().value);
    advance();
    return JsonValue(value);
}

JsonValue Parser::parseBool() {
    bool value = (current().type == TokenType::True);
    advance();
    return JsonValue(value);
}

JsonValue Parser::parseNull() {
    advance();
    return JsonValue(nullptr);
}

// Статические методы
JsonValue Parser::parseString(const std::string& jsonStr) {
    Lexer lexer(jsonStr);
    std::vector<Token> tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    return parser.parse();
}

JsonValue Parser::parseFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw JsonException("Не удалось открыть файл: " + filename);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return parseString(buffer.str());
}

JsonValue Parser::parseFileWithProgress(const std::string& filename, ProgressCallback callback) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw JsonException("Не удалось открыть файл: " + filename);
    }

    // Получаем размер файла
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Читаем файл с прогрессом
    std::string content;
    content.reserve(fileSize);

    const size_t chunkSize = 1024 * 1024; // 1 МБ чанки
    std::vector<char> buffer(chunkSize);
    size_t totalRead = 0;

    while (file.good()) {
        file.read(buffer.data(), chunkSize);
        std::streamsize bytesRead = file.gcount();

        if (bytesRead > 0) {
            content.append(buffer.data(), bytesRead);
            totalRead += bytesRead;

            if (callback) {
                callback(totalRead, fileSize);
            }
        }
    }

    file.close();

    // Токенизация
    Lexer lexer(content);
    std::vector<Token> tokens = lexer.tokenize();

    // Парсинг с прогрессом
    Parser parser(std::move(tokens));
    parser.setProgressCallback(callback);
    return parser.parse();
}

} // namespace json
