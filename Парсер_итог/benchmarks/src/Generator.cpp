#include "Generator.hpp"
#include <fstream>
#include <sstream>
#include <ctime>
#include <algorithm>

namespace json {

// Списки для генерации случайных данных
static const std::vector<std::string> FIRST_NAMES = {
    "Алексей", "Мария", "Иван", "Елена", "Дмитрий", "Анна", "Сергей", "Ольга",
    "Андрей", "Наталья", "Михаил", "Екатерина", "Павел", "Татьяна", "Николай"
};

static const std::vector<std::string> LAST_NAMES = {
    "Иванов", "Петров", "Сидоров", "Козлов", "Новиков", "Морозов", "Волков",
    "Соколов", "Лебедев", "Кузнецов", "Попов", "Смирнов", "Федоров", "Орлов"
};

static const std::vector<std::string> CITIES = {
    "Москва", "Санкт-Петербург", "Новосибирск", "Екатеринбург", "Казань",
    "Нижний Новгород", "Челябинск", "Самара", "Омск", "Ростов-на-Дону"
};

static const std::vector<std::string> PRODUCTS = {
    "Ноутбук", "Смартфон", "Планшет", "Наушники", "Клавиатура", "Мышь",
    "Монитор", "Принтер", "Роутер", "Камера", "Колонки", "Микрофон"
};

static const std::vector<std::string> KEYS = {
    "id", "name", "email", "age", "active", "score", "price", "count",
    "title", "description", "status", "created", "updated", "type",
    "value", "data", "items", "users", "config", "settings", "meta"
};

Generator::Generator(unsigned int seed)
    : m_currentLine(1), m_currentColumn(1) {
    if (seed == 0) {
        m_rng.seed(static_cast<unsigned int>(std::time(nullptr)));
    } else {
        m_rng.seed(seed);
    }
}

void Generator::setOptions(const GeneratorOptions& options) {
    m_options = options;
}

int Generator::randomInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(m_rng);
}

double Generator::randomDouble(double min, double max) {
    std::uniform_real_distribution<double> dist(min, max);
    return dist(m_rng);
}

bool Generator::randomBool() {
    return randomInt(0, 1) == 1;
}

bool Generator::shouldGenerateError() {
    return randomInt(1, 100) <= m_options.errorProbability;
}

std::string Generator::randomString(int minLen, int maxLen) {
    int type = randomInt(0, 4);

    switch (type) {
        case 0: // Имя
            return FIRST_NAMES[randomInt(0, FIRST_NAMES.size() - 1)];
        case 1: // Фамилия
            return LAST_NAMES[randomInt(0, LAST_NAMES.size() - 1)];
        case 2: // Город
            return CITIES[randomInt(0, CITIES.size() - 1)];
        case 3: // Продукт
            return PRODUCTS[randomInt(0, PRODUCTS.size() - 1)];
        default: { // Случайная строка
            static const std::string chars = "abcdefghijklmnopqrstuvwxyz0123456789";
            int len = randomInt(minLen, maxLen);
            std::string result;
            for (int i = 0; i < len; ++i) {
                result += chars[randomInt(0, chars.size() - 1)];
            }
            return result;
        }
    }
}

std::string Generator::randomKey() {
    if (randomBool()) {
        return KEYS[randomInt(0, KEYS.size() - 1)] + std::to_string(randomInt(1, 99));
    }
    return KEYS[randomInt(0, KEYS.size() - 1)];
}

void Generator::updatePosition(const std::string& str) {
    for (char c : str) {
        if (c == '\n') {
            m_currentLine++;
            m_currentColumn = 1;
        } else {
            m_currentColumn++;
        }
    }
}

std::string Generator::generateString() {
    std::string str = randomString();
    // Иногда добавляем escape-последовательности
    if (randomInt(0, 10) == 0) {
        int escapeType = randomInt(0, 3);
        switch (escapeType) {
            case 0: str += "\\n"; break;
            case 1: str += "\\t"; break;
            case 2: str += "\\\""; break;
            case 3: str += "\\\\"; break;
        }
    }
    return "\"" + str + "\"";
}

std::string Generator::generateNumber() {
    int type = randomInt(0, 3);
    std::ostringstream oss;

    switch (type) {
        case 0: // Целое положительное
            oss << randomInt(0, 10000);
            break;
        case 1: // Целое отрицательное
            oss << -randomInt(1, 10000);
            break;
        case 2: // Дробное
            oss << std::fixed;
            oss.precision(randomInt(1, 4));
            oss << randomDouble(-1000.0, 1000.0);
            break;
        case 3: // Научная нотация
            oss << randomDouble(1.0, 9.99) << "e" << randomInt(-10, 10);
            break;
    }

    return oss.str();
}

std::string Generator::generateBool() {
    return randomBool() ? "true" : "false";
}

std::string Generator::generateNull() {
    return "null";
}

std::string Generator::generateValue(int depth) {
    // На максимальной глубине генерируем только примитивы
    if (depth >= m_options.maxDepth) {
        int type = randomInt(0, 3);
        switch (type) {
            case 0: return generateString();
            case 1: return generateNumber();
            case 2: return generateBool();
            case 3: return generateNull();
        }
    }

    // Выбираем тип значения
    int type = randomInt(0, 5);
    switch (type) {
        case 0: return generateObject(depth + 1);
        case 1: return generateArray(depth + 1);
        case 2: return generateString();
        case 3: return generateNumber();
        case 4: return generateBool();
        case 5: return generateNull();
    }

    return generateNull();
}

std::string Generator::generateObject(int depth) {
    std::ostringstream oss;
    int keyCount = randomInt(m_options.minObjectKeys, m_options.maxObjectKeys);
    std::vector<std::string> usedKeys;

    if (m_options.compactOutput) {
        oss << "{";
        for (int i = 0; i < keyCount; ++i) {
            std::string key;
            do {
                key = randomKey();
            } while (std::find(usedKeys.begin(), usedKeys.end(), key) != usedKeys.end());
            usedKeys.push_back(key);

            if (i > 0) {
                oss << ",";
            }
            oss << "\"" << key << "\":" << generateValue(depth);
        }
        oss << "}";
        return oss.str();
    }

    oss << "{\n";

    for (int i = 0; i < keyCount; ++i) {
        // Генерируем уникальный ключ
        std::string key;
        do {
            key = randomKey();
        } while (std::find(usedKeys.begin(), usedKeys.end(), key) != usedKeys.end());
        usedKeys.push_back(key);

        // Отступы
        std::string indent(depth * 2, ' ');
        oss << indent << "\"" << key << "\": " << generateValue(depth);

        if (i < keyCount - 1) {
            oss << ",";
        }
        oss << "\n";
    }

    std::string closeIndent((depth - 1) * 2, ' ');
    oss << closeIndent << "}";

    return oss.str();
}

std::string Generator::generateArray(int depth) {
    std::ostringstream oss;
    int size = randomInt(m_options.minArraySize, m_options.maxArraySize);

    if (m_options.compactOutput) {
        oss << "[";
        for (int i = 0; i < size; ++i) {
            if (i > 0) {
                oss << ",";
            }
            oss << generateValue(depth);
        }
        oss << "]";
        return oss.str();
    }

    oss << "[\n";

    for (int i = 0; i < size; ++i) {
        std::string indent(depth * 2, ' ');
        oss << indent << generateValue(depth);

        if (i < size - 1) {
            oss << ",";
        }
        oss << "\n";
    }

    std::string closeIndent((depth - 1) * 2, ' ');
    oss << closeIndent << "]";

    return oss.str();
}

ErrorType Generator::selectRandomError() {
    int type = randomInt(1, 10);
    return static_cast<ErrorType>(type);
}

std::string Generator::getErrorDescription(ErrorType type) {
    switch (type) {
        case ErrorType::MissingComma: return "Пропущенная запятая";
        case ErrorType::MissingColon: return "Пропущенное двоеточие";
        case ErrorType::MissingQuote: return "Незакрытая кавычка";
        case ErrorType::MissingBracket: return "Незакрытая скобка";
        case ErrorType::ExtraComma: return "Лишняя запятая (trailing comma)";
        case ErrorType::InvalidNumber: return "Некорректное число";
        case ErrorType::InvalidKeyword: return "Неверное ключевое слово";
        case ErrorType::UnquotedKey: return "Ключ без кавычек";
        case ErrorType::SingleQuotes: return "Одинарные кавычки";
        case ErrorType::TrailingData: return "Лишние данные после JSON";
        default: return "Неизвестная ошибка";
    }
}

std::string Generator::injectError(const std::string& json, ErrorType errorType) {
    std::string result = json;
    size_t pos;

    GeneratedError error;
    error.type = errorType;
    error.description = getErrorDescription(errorType);

    switch (errorType) {
        case ErrorType::MissingComma:
            // Удаляем случайную запятую
            pos = result.find(",\n");
            if (pos != std::string::npos && pos > 10) {
                // Находим случайную позицию запятой
                std::vector<size_t> commaPositions;
                size_t searchPos = 0;
                while ((searchPos = result.find(",\n", searchPos)) != std::string::npos) {
                    commaPositions.push_back(searchPos);
                    searchPos++;
                }
                if (!commaPositions.empty()) {
                    pos = commaPositions[randomInt(0, commaPositions.size() - 1)];
                    result.erase(pos, 1);
                }
            }
            break;

        case ErrorType::MissingColon:
            // Удаляем случайное двоеточие
            pos = result.find("\": ");
            if (pos != std::string::npos) {
                std::vector<size_t> colonPositions;
                size_t searchPos = 0;
                while ((searchPos = result.find("\": ", searchPos)) != std::string::npos) {
                    colonPositions.push_back(searchPos + 1);
                    searchPos++;
                }
                if (!colonPositions.empty()) {
                    pos = colonPositions[randomInt(0, colonPositions.size() - 1)];
                    result.erase(pos, 1);
                }
            }
            break;

        case ErrorType::MissingQuote:
            // Удаляем закрывающую кавычку у строки
            pos = result.rfind("\"");
            if (pos > 5) {
                // Ищем строковое значение
                size_t searchPos = randomInt(5, result.size() / 2);
                pos = result.find("\",", searchPos);
                if (pos == std::string::npos) {
                    pos = result.find("\"\n", searchPos);
                }
                if (pos != std::string::npos) {
                    result.erase(pos, 1);
                }
            }
            break;

        case ErrorType::MissingBracket:
            // Удаляем закрывающую скобку
            if (randomBool()) {
                pos = result.rfind("}");
            } else {
                pos = result.rfind("]");
            }
            if (pos != std::string::npos && pos > 5) {
                result.erase(pos, 1);
            }
            break;

        case ErrorType::ExtraComma:
            // Добавляем запятую перед закрывающей скобкой
            pos = result.rfind("\n}");
            if (pos == std::string::npos) {
                pos = result.rfind("\n]");
            }
            if (pos != std::string::npos) {
                result.insert(pos, ",");
            }
            break;

        case ErrorType::InvalidNumber:
            // Заменяем число на некорректное
            pos = result.find(": ");
            if (pos != std::string::npos) {
                std::vector<size_t> numPositions;
                size_t searchPos = 0;
                while ((searchPos = result.find(": ", searchPos)) != std::string::npos) {
                    // Проверяем, что это число
                    if (searchPos + 2 < result.size() &&
                        (std::isdigit(result[searchPos + 2]) || result[searchPos + 2] == '-')) {
                        numPositions.push_back(searchPos + 2);
                    }
                    searchPos++;
                }
                if (!numPositions.empty()) {
                    pos = numPositions[randomInt(0, numPositions.size() - 1)];
                    // Находим конец числа
                    size_t endPos = pos;
                    while (endPos < result.size() &&
                           (std::isdigit(result[endPos]) || result[endPos] == '.' ||
                            result[endPos] == '-' || result[endPos] == 'e' || result[endPos] == 'E')) {
                        endPos++;
                    }
                    result.replace(pos, endPos - pos, "1.2.3.4");
                }
            }
            break;

        case ErrorType::InvalidKeyword:
            // Заменяем true/false/null на неправильный вариант
            pos = result.find("true");
            if (pos != std::string::npos) {
                result.replace(pos, 4, "True");
            } else {
                pos = result.find("false");
                if (pos != std::string::npos) {
                    result.replace(pos, 5, "False");
                } else {
                    pos = result.find("null");
                    if (pos != std::string::npos) {
                        result.replace(pos, 4, "Null");
                    }
                }
            }
            break;

        case ErrorType::UnquotedKey:
            // Удаляем кавычки вокруг ключа
            pos = result.find("  \"");
            if (pos != std::string::npos) {
                size_t endQuote = result.find("\":", pos + 3);
                if (endQuote != std::string::npos) {
                    result.erase(endQuote, 1);
                    result.erase(pos + 2, 1);
                }
            }
            break;

        case ErrorType::SingleQuotes:
            // Заменяем двойные кавычки на одинарные
            pos = result.find("\"", 5);
            if (pos != std::string::npos) {
                result[pos] = '\'';
                size_t nextQuote = result.find("\"", pos + 1);
                if (nextQuote != std::string::npos) {
                    result[nextQuote] = '\'';
                }
            }
            break;

        case ErrorType::TrailingData:
            // Добавляем мусор в конец
            result += "\n{\"extra\": \"data\"}";
            break;

        default:
            break;
    }

    // Вычисляем примерную позицию ошибки
    int line = 1, col = 1;
    for (size_t i = 0; i < std::min(pos, result.size()); ++i) {
        if (result[i] == '\n') {
            line++;
            col = 1;
        } else {
            col++;
        }
    }
    error.line = line;
    error.column = col;

    m_errors.push_back(error);

    return result;
}

std::string Generator::generate() {
    m_errors.clear();
    m_currentLine = 1;
    m_currentColumn = 1;

    std::string json;

    // Генерируем корневой элемент (объект или массив)
    if (randomBool()) {
        json = generateObject(1);
    } else {
        json = generateArray(1);
    }

    // Если нужно внести ошибки
    if (m_options.errorProbability > 0 && shouldGenerateError()) {
        ErrorType errorType = selectRandomError();
        json = injectError(json, errorType);
    }

    return json;
}

std::string Generator::generateWithErrors(int errorCount) {
    m_errors.clear();
    m_currentLine = 1;
    m_currentColumn = 1;

    // Временно отключаем случайные ошибки
    int savedProbability = m_options.errorProbability;
    m_options.errorProbability = 0;

    std::string json;
    if (randomBool()) {
        json = generateObject(1);
    } else {
        json = generateArray(1);
    }

    // Вносим заданное количество ошибок
    for (int i = 0; i < errorCount; ++i) {
        ErrorType errorType = selectRandomError();
        json = injectError(json, errorType);
    }

    m_options.errorProbability = savedProbability;
    return json;
}

bool Generator::saveToFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    file << content;
    return file.good();
}

std::string Generator::quickGenerate(int depth, int errorProbability, unsigned int seed) {
    Generator gen(seed);
    GeneratorOptions opts;
    opts.maxDepth = depth;
    opts.errorProbability = errorProbability;
    gen.setOptions(opts);
    return gen.generate();
}

} // namespace json
