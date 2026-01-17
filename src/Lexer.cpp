#include "Lexer.hpp"
#include <cctype>
#include <sstream>
#include <iomanip>

namespace json {

Lexer::Lexer(const std::string& input)
    : m_input(input), m_pos(0), m_line(1), m_column(1) {}

char Lexer::current() const {
    if (isAtEnd()) return '\0';
    return m_input[m_pos];
}

char Lexer::peek(size_t offset) const {
    if (m_pos + offset >= m_input.size()) return '\0';
    return m_input[m_pos + offset];
}

void Lexer::advance() {
    if (!isAtEnd()) {
        if (m_input[m_pos] == '\n') {
            m_line++;
            m_column = 1;
        } else {
            m_column++;
        }
        m_pos++;
    }
}

bool Lexer::isAtEnd() const {
    return m_pos >= m_input.size();
}

void Lexer::skipWhitespace() {
    while (!isAtEnd() && std::isspace(static_cast<unsigned char>(current()))) {
        advance();
    }
}

Token Lexer::parseString() {
    size_t startLine = m_line;
    size_t startColumn = m_column;

    advance(); // пропускаем открывающую кавычку

    std::string result;
    while (!isAtEnd() && current() != '"') {
        if (current() == '\\') {
            advance(); // пропускаем backslash
            if (isAtEnd()) {
                throw LexerException("Неожиданный конец строки после escape-символа", m_line, m_column);
            }

            switch (current()) {
                case '"':  result += '"';  break;
                case '\\': result += '\\'; break;
                case '/':  result += '/';  break;
                case 'b':  result += '\b'; break;
                case 'f':  result += '\f'; break;
                case 'n':  result += '\n'; break;
                case 'r':  result += '\r'; break;
                case 't':  result += '\t'; break;
                case 'u': {
                    advance(); // пропускаем 'u'
                    char32_t codePoint = parseUnicodeEscape();

                    // Проверка на суррогатную пару
                    if (codePoint >= 0xD800 && codePoint <= 0xDBFF) {
                        // Это high surrogate, ищем low surrogate
                        if (current() == '\\' && peek() == 'u') {
                            advance(); // пропускаем '\'
                            advance(); // пропускаем 'u'
                            char32_t lowSurrogate = parseUnicodeEscape();
                            if (lowSurrogate >= 0xDC00 && lowSurrogate <= 0xDFFF) {
                                // Вычисляем реальный code point
                                codePoint = 0x10000 + ((codePoint - 0xD800) << 10) + (lowSurrogate - 0xDC00);
                            } else {
                                throw LexerException("Неверный low surrogate в unicode escape", m_line, m_column);
                            }
                        } else {
                            throw LexerException("Ожидался low surrogate после high surrogate", m_line, m_column);
                        }
                    }

                    result += codePointToUTF8(codePoint);
                    continue; // не вызываем advance() в конце switch
                }
                default:
                    throw LexerException("Неизвестная escape-последовательность: \\" +
                                        std::string(1, current()), m_line, m_column);
            }
        } else if (static_cast<unsigned char>(current()) < 0x20) {
            throw LexerException("Управляющий символ в строке не допускается", m_line, m_column);
        } else {
            result += current();
        }
        advance();
    }

    if (isAtEnd()) {
        throw LexerException("Незакрытая строка", startLine, startColumn);
    }

    advance(); // пропускаем закрывающую кавычку
    return Token(TokenType::String, result, startLine, startColumn);
}

char32_t Lexer::parseUnicodeEscape() {
    std::string hex;
    for (int i = 0; i < 4; i++) {
        if (isAtEnd() || !std::isxdigit(static_cast<unsigned char>(current()))) {
            throw LexerException("Ожидалось 4 шестнадцатеричных цифры в unicode escape", m_line, m_column);
        }
        hex += current();
        advance();
    }
    return static_cast<char32_t>(std::stoul(hex, nullptr, 16));
}

std::string Lexer::codePointToUTF8(char32_t cp) {
    std::string result;
    if (cp <= 0x7F) {
        result += static_cast<char>(cp);
    } else if (cp <= 0x7FF) {
        result += static_cast<char>(0xC0 | (cp >> 6));
        result += static_cast<char>(0x80 | (cp & 0x3F));
    } else if (cp <= 0xFFFF) {
        result += static_cast<char>(0xE0 | (cp >> 12));
        result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (cp & 0x3F));
    } else if (cp <= 0x10FFFF) {
        result += static_cast<char>(0xF0 | (cp >> 18));
        result += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
        result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (cp & 0x3F));
    }
    return result;
}

Token Lexer::parseNumber() {
    size_t startLine = m_line;
    size_t startColumn = m_column;
    std::string numStr;

    // Опциональный минус
    if (current() == '-') {
        numStr += current();
        advance();
    }

    // Целая часть
    if (current() == '0') {
        numStr += current();
        advance();
    } else if (std::isdigit(static_cast<unsigned char>(current()))) {
        while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(current()))) {
            numStr += current();
            advance();
        }
    } else {
        throw LexerException("Ожидалась цифра в числе", m_line, m_column);
    }

    // Дробная часть
    if (current() == '.') {
        numStr += current();
        advance();
        if (!std::isdigit(static_cast<unsigned char>(current()))) {
            throw LexerException("Ожидалась цифра после точки", m_line, m_column);
        }
        while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(current()))) {
            numStr += current();
            advance();
        }
    }

    // Экспонента
    if (current() == 'e' || current() == 'E') {
        numStr += current();
        advance();
        if (current() == '+' || current() == '-') {
            numStr += current();
            advance();
        }
        if (!std::isdigit(static_cast<unsigned char>(current()))) {
            throw LexerException("Ожидалась цифра в экспоненте", m_line, m_column);
        }
        while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(current()))) {
            numStr += current();
            advance();
        }
    }

    return Token(TokenType::Number, numStr, startLine, startColumn);
}

Token Lexer::parseKeyword() {
    size_t startLine = m_line;
    size_t startColumn = m_column;
    std::string word;

    while (!isAtEnd() && std::isalpha(static_cast<unsigned char>(current()))) {
        word += current();
        advance();
    }

    if (word == "true") {
        return Token(TokenType::True, word, startLine, startColumn);
    } else if (word == "false") {
        return Token(TokenType::False, word, startLine, startColumn);
    } else if (word == "null") {
        return Token(TokenType::Null, word, startLine, startColumn);
    } else {
        throw LexerException("Неизвестное ключевое слово: " + word, startLine, startColumn);
    }
}

Token Lexer::nextToken() {
    skipWhitespace();

    if (isAtEnd()) {
        return Token(TokenType::EndOfFile, "", m_line, m_column);
    }

    size_t startLine = m_line;
    size_t startColumn = m_column;
    char c = current();

    switch (c) {
        case '{':
            advance();
            return Token(TokenType::LeftBrace, "{", startLine, startColumn);
        case '}':
            advance();
            return Token(TokenType::RightBrace, "}", startLine, startColumn);
        case '[':
            advance();
            return Token(TokenType::LeftBracket, "[", startLine, startColumn);
        case ']':
            advance();
            return Token(TokenType::RightBracket, "]", startLine, startColumn);
        case ':':
            advance();
            return Token(TokenType::Colon, ":", startLine, startColumn);
        case ',':
            advance();
            return Token(TokenType::Comma, ",", startLine, startColumn);
        case '"':
            return parseString();
        case '-':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            return parseNumber();
        default:
            if (std::isalpha(static_cast<unsigned char>(c))) {
                return parseKeyword();
            }
            throw LexerException("Неожиданный символ: " + std::string(1, c), m_line, m_column);
    }
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    Token token = nextToken();
    while (token.type != TokenType::EndOfFile) {
        tokens.push_back(token);
        token = nextToken();
    }
    tokens.push_back(token); // добавляем EndOfFile
    return tokens;
}

std::string tokenTypeName(TokenType type) {
    switch (type) {
        case TokenType::LeftBrace: return "LeftBrace";
        case TokenType::RightBrace: return "RightBrace";
        case TokenType::LeftBracket: return "LeftBracket";
        case TokenType::RightBracket: return "RightBracket";
        case TokenType::Colon: return "Colon";
        case TokenType::Comma: return "Comma";
        case TokenType::String: return "String";
        case TokenType::Number: return "Number";
        case TokenType::True: return "True";
        case TokenType::False: return "False";
        case TokenType::Null: return "Null";
        case TokenType::EndOfFile: return "EndOfFile";
        default: return "Unknown";
    }
}

} // namespace json
