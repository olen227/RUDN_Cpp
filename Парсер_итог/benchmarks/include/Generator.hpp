#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include <string>
#include <vector>
#include <random>

namespace json {

// Типы ошибок, которые может генерировать генератор
enum class ErrorType {
    None,
    MissingComma,           // Пропущенная запятая
    MissingColon,           // Пропущенное двоеточие
    MissingQuote,           // Незакрытая кавычка
    MissingBracket,         // Незакрытая скобка [ или {
    ExtraComma,             // Лишняя запятая (trailing comma)
    InvalidNumber,          // Некорректное число (например, 1.2.3)
    InvalidKeyword,         // Неправильное ключевое слово (True вместо true)
    UnquotedKey,            // Ключ без кавычек
    SingleQuotes,           // Одинарные кавычки вместо двойных
    TrailingData            // Лишние данные после JSON
};

// Информация о сгенерированной ошибке
struct GeneratedError {
    ErrorType type;
    int line;
    int column;
    std::string description;
};

// Настройки генерации
struct GeneratorOptions {
    int minDepth = 1;           // Минимальная глубина вложенности
    int maxDepth = 5;           // Максимальная глубина вложенности
    int minArraySize = 1;       // Минимальный размер массива
    int maxArraySize = 10;      // Максимальный размер массива
    int minObjectKeys = 1;      // Минимальное количество ключей в объекте
    int maxObjectKeys = 8;      // Максимальное количество ключей в объекте
    int errorProbability = 0;   // Вероятность ошибки (0-100%)
    bool generateComments = false; // Генерировать комментарии (невалидно для JSON)
    bool compactOutput = false; // Компактный вывод без переносов строк
};

// Класс генератора JSON
class Generator {
private:
    std::mt19937 m_rng;
    GeneratorOptions m_options;
    std::vector<GeneratedError> m_errors;
    int m_currentLine;
    int m_currentColumn;

    // Вспомогательные методы генерации
    std::string generateValue(int depth);
    std::string generateObject(int depth);
    std::string generateArray(int depth);
    std::string generateString();
    std::string generateNumber();
    std::string generateBool();
    std::string generateNull();

    // Генерация случайных данных
    std::string randomString(int minLen = 3, int maxLen = 15);
    std::string randomKey();
    int randomInt(int min, int max);
    double randomDouble(double min, double max);
    bool randomBool();
    bool shouldGenerateError();

    // Внесение ошибок
    std::string injectError(const std::string& json, ErrorType errorType);
    ErrorType selectRandomError();
    std::string getErrorDescription(ErrorType type);

    // Обновление позиции
    void updatePosition(const std::string& str);

public:
    explicit Generator(unsigned int seed = 0);

    // Установка опций
    void setOptions(const GeneratorOptions& options);
    GeneratorOptions& getOptions() { return m_options; }

    // Генерация JSON
    std::string generate();

    // Генерация с заданным количеством ошибок
    std::string generateWithErrors(int errorCount);

    // Получение списка сгенерированных ошибок
    const std::vector<GeneratedError>& getGeneratedErrors() const { return m_errors; }

    // Очистка ошибок
    void clearErrors() { m_errors.clear(); }

    // Сохранение в файл
    bool saveToFile(const std::string& filename, const std::string& content);

    // Статический метод для быстрой генерации
    static std::string quickGenerate(int depth = 3, int errorProbability = 0, unsigned int seed = 0);
};

} // namespace json

#endif // GENERATOR_HPP
