#include "Parser.hpp"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <thread>
#include <vector>
#include <future>
#include <mutex>

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

// Вспомогательный метод для разбиения токенов массива на чанки
std::vector<std::pair<size_t, size_t>> Parser::splitArrayTokens(
    const std::vector<Token>& tokens, size_t threadCount) {

    std::vector<std::pair<size_t, size_t>> chunks;

    // Проверяем, что это массив
    if (tokens.empty() || tokens[0].type != TokenType::LeftBracket) {
        // Не массив - возвращаем весь диапазон как один чанк
        chunks.emplace_back(0, tokens.size());
        return chunks;
    }

    // Ищем границы элементов массива (начало и конец каждого элемента)
    std::vector<std::pair<size_t, size_t>> elements; // пары (start, end) для каждого элемента
    int depth = 0;
    size_t elementStart = 0;
    bool inElement = false;

    for (size_t i = 0; i < tokens.size(); ++i) {
        const Token& token = tokens[i];

        if (token.type == TokenType::LeftBracket) {
            if (depth == 0 && i + 1 < tokens.size() && tokens[i + 1].type != TokenType::RightBracket) {
                // Начало массива - следующий токен это начало первого элемента
                elementStart = i + 1;
                inElement = true;
            }
            depth++;
        } else if (token.type == TokenType::RightBracket) {
            depth--;
            if (depth == 0 && inElement) {
                // Конец массива - заканчиваем последний элемент
                elements.emplace_back(elementStart, i);
                inElement = false;
            }
        } else if (token.type == TokenType::LeftBrace) {
            depth++;
        } else if (token.type == TokenType::RightBrace) {
            depth--;
        } else if (token.type == TokenType::Comma && depth == 1 && inElement) {
            // Запятая на уровне массива - конец текущего элемента
            elements.emplace_back(elementStart, i); // не включаем запятую
            if (i + 1 < tokens.size() && tokens[i + 1].type != TokenType::RightBracket) {
                elementStart = i + 1; // начало следующего элемента
            } else {
                inElement = false;
            }
        }
    }

    if (elements.empty()) {
        // Пустой массив или не удалось разбить
        chunks.emplace_back(0, tokens.size());
        return chunks;
    }

    // Разбиваем элементы на чанки
    size_t elementsPerChunk = (elements.size() + threadCount - 1) / threadCount;
    if (elementsPerChunk < 1) elementsPerChunk = 1;

    // Создаём чанки, группируя элементы
    for (size_t i = 0; i < elements.size(); i += elementsPerChunk) {
        size_t chunkStartIdx = i;
        size_t chunkEndIdx = std::min(i + elementsPerChunk, elements.size());

        // Начало чанка - начало первого элемента в группе
        // Конец чанка - конец последнего элемента в группе
        size_t start = elements[chunkStartIdx].first;
        size_t end = elements[chunkEndIdx - 1].second;

        chunks.emplace_back(start, end);
    }

    return chunks;
}

// Парсинг диапазона токенов
JsonValue Parser::parseTokenRange(const std::vector<Token>& allTokens, size_t start, size_t end) {
    // Создаём подмножество токенов
    std::vector<Token> chunkTokens;
    chunkTokens.reserve(end - start + 3); // +3 для скобок массива и EOF

    // Добавляем открывающую скобку массива
    chunkTokens.emplace_back(TokenType::LeftBracket, "[", 0, 0);

    // Копируем токены диапазона (элементы уже без запятых на концах)
    for (size_t i = start; i < end && i < allTokens.size(); ++i) {
        chunkTokens.push_back(allTokens[i]);
    }

    // Добавляем закрывающую скобку массива
    chunkTokens.emplace_back(TokenType::RightBracket, "]", 0, 0);
    chunkTokens.emplace_back(TokenType::EndOfFile, "", 0, 0);

    // Парсим этот чанк
    Parser parser(std::move(chunkTokens));
    return parser.parse();
}

// Разбиение содержимого на текстовые чанки по границам элементов массива
std::vector<std::pair<size_t, size_t>> Parser::splitContentIntoChunks(
    const std::string& content, size_t threadCount) {

    std::vector<std::pair<size_t, size_t>> chunks;

    // Находим начало массива
    size_t start = content.find('[');
    if (start == std::string::npos) {
        // Не массив - возвращаем весь контент
        chunks.emplace_back(0, content.size());
        return chunks;
    }

    start++; // Пропускаем '['

    // Пропускаем пробелы
    while (start < content.size() && std::isspace(static_cast<unsigned char>(content[start]))) {
        start++;
    }

    // Находим все позиции элементов массива
    std::vector<std::pair<size_t, size_t>> elements; // (start, end) для каждого элемента
    int depth = 0;
    bool inString = false;
    bool escaped = false;
    size_t elementStart = start;
    bool inElement = true;

    for (size_t i = start; i < content.size(); ++i) {
        char c = content[i];

        if (escaped) {
            escaped = false;
            continue;
        }

        if (c == '\\' && inString) {
            escaped = true;
            continue;
        }

        if (c == '"' && !escaped) {
            inString = !inString;
            continue;
        }

        if (inString) continue;

        if (c == '{' || c == '[') {
            depth++;
        } else if (c == '}' || c == ']') {
            depth--;
            if (depth < 0) {
                // Конец массива
                if (inElement && i > elementStart) {
                    elements.emplace_back(elementStart, i);
                }
                break;
            }
        } else if (c == ',' && depth == 0) {
            // Запятая на уровне массива - конец элемента
            if (inElement) {
                elements.emplace_back(elementStart, i);
                // Пропускаем пробелы после запятой
                size_t nextStart = i + 1;
                while (nextStart < content.size() &&
                       std::isspace(static_cast<unsigned char>(content[nextStart]))) {
                    nextStart++;
                }
                elementStart = nextStart;
            }
        }
    }

    if (elements.empty()) {
        // Пустой массив или не удалось разбить
        chunks.emplace_back(0, content.size());
        return chunks;
    }

    // Распределяем элементы по чанкам
    size_t elementsPerChunk = (elements.size() + threadCount - 1) / threadCount;
    if (elementsPerChunk < 1) elementsPerChunk = 1;

    for (size_t i = 0; i < elements.size(); i += elementsPerChunk) {
        size_t endIdx = std::min(i + elementsPerChunk, elements.size());

        size_t chunkStart = elements[i].first;
        size_t chunkEnd = elements[endIdx - 1].second;

        chunks.emplace_back(chunkStart, chunkEnd);
    }

    return chunks;
}

// Многопоточный парсинг файла
JsonValue Parser::parseFileParallel(const std::string& filename, unsigned int threadCount,
                                    ProgressCallback callback) {
    // Определяем количество потоков
    if (threadCount == 0) {
        threadCount = std::thread::hardware_concurrency();
        if (threadCount == 0) threadCount = 1;
    }

    // Читаем файл
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw JsonException("Не удалось открыть файл: " + filename);
    }

    // Получаем размер файла
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Читаем содержимое
    std::string content;
    content.resize(fileSize);
    file.read(&content[0], fileSize);
    file.close();

    if (callback) callback(fileSize / 10, fileSize); // 10% - чтение завершено

    // Проверяем, является ли JSON массивом на первом уровне
    // Быстрая проверка первого непробельного символа
    size_t firstNonSpace = 0;
    while (firstNonSpace < content.size() && std::isspace(static_cast<unsigned char>(content[firstNonSpace]))) {
        firstNonSpace++;
    }

    if (firstNonSpace >= content.size() || content[firstNonSpace] != '[') {
        // Не массив - используем обычный последовательный парсинг
        Lexer lexer(content);
        std::vector<Token> tokens = lexer.tokenize();
        Parser parser(std::move(tokens));
        if (callback) callback(fileSize, fileSize);
        return parser.parse();
    }

    // Разбиваем содержимое на текстовые чанки по границам элементов массива
    auto textChunks = splitContentIntoChunks(content, threadCount);

    if (textChunks.size() == 1) {
        // Не удалось разбить - используем последовательный парсинг
        Lexer lexer(content);
        std::vector<Token> tokens = lexer.tokenize();
        Parser parser(std::move(tokens));
        if (callback) callback(fileSize, fileSize);
        return parser.parse();
    }

    // ПАРАЛЛЕЛЬНАЯ ТОКЕНИЗАЦИЯ И ПАРСИНГ
    std::vector<std::future<JsonArray>> futures;
    futures.reserve(textChunks.size());

    std::mutex progressMutex;
    std::atomic<size_t> completedChunks{0};
    size_t totalChunks = textChunks.size();

    for (const auto& chunk : textChunks) {
        futures.push_back(std::async(std::launch::async,
            [&content, chunk, callback, &progressMutex, &completedChunks, totalChunks, fileSize]() {
            // Извлекаем текстовый чанк
            std::string chunkText = content.substr(chunk.first, chunk.second - chunk.first);

            // Оборачиваем в массив для валидности
            std::string wrappedChunk = "[" + chunkText + "]";

            // Токенизация чанка (параллельно!)
            Lexer lexer(wrappedChunk);
            std::vector<Token> tokens = lexer.tokenize();

            // Парсинг чанка (параллельно!)
            Parser parser(std::move(tokens));
            JsonValue result = parser.parse();

            // Обновляем прогресс
            completedChunks++;
            if (callback) {
                std::lock_guard<std::mutex> lock(progressMutex);
                size_t progress = fileSize / 10 + (fileSize * 9 * completedChunks) / (10 * totalChunks);
                callback(progress, fileSize);
            }

            // Возвращаем массив элементов
            if (result.isArray()) {
                return result.asArray();
            }
            return JsonArray();
        }));
    }

    // Собираем результаты
    JsonArray finalArray;
    for (auto& future : futures) {
        JsonArray chunkArray = future.get();
        for (const JsonValue& elem : chunkArray) {
            finalArray.push_back(elem);
        }
    }

    if (callback) callback(fileSize, fileSize); // 100% - готово

    return JsonValue(std::move(finalArray));
}

} // namespace json
