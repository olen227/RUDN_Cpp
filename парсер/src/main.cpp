#include "JsonValue.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Serializer.hpp"
#include "Generator.hpp"
#include "Validator.hpp"
#include "ParallelProcessor.hpp"
#include "SystemInfo.hpp"
#include "ProgressBar.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <iomanip>
#include <chrono>
#include <thread>
#include <filesystem>
#include <cctype>
#include <atomic>

namespace fs = std::filesystem;

using namespace json;

// Структура для хранения метрик выполнения
struct PerformanceMetrics {
    double parseTimeMs = 0;
    double serializeTimeMs = 0;
    double searchTimeMs = 0;
    size_t fileSize = 0;
    size_t tokenCount = 0;
    int maxDepth = 0;
};

PerformanceMetrics g_metrics;

struct StreamParseResult {
    bool success = true;
    size_t valueCount = 0;
    size_t tokenCount = 0;
    size_t lineCount = 0;
    int maxDepth = 0;
    std::string errorMessage;
    size_t errorLine = 0;
};

struct TolerantParseResult {
    JsonArray validElements;           // Успешно загруженные элементы
    std::vector<std::string> errors;   // Список ошибок (строка + описание)
    size_t totalLines = 0;
    size_t successCount = 0;
    size_t errorCount = 0;
    int maxDepth = 0;
};

// Глобальные переменные для текущего состояния
JsonValue g_currentJson;
std::string g_currentFile;
bool g_isModified = false;
bool g_isStreamMode = false; // Флаг потокового режима

// Папка для данных (относительно корня проекта)
const std::string DATA_DIR = "data";
const size_t STREAMING_THRESHOLD_BYTES = 256ULL * 1024 * 1024;

fs::path findProjectRoot() {
    fs::path current = fs::current_path();
    fs::path path = current;

    for (int i = 0; i < 4; ++i) {
        if (fs::exists(path / "CMakeLists.txt")) {
            return path;
        }
        if (!path.has_parent_path()) {
            break;
        }
        path = path.parent_path();
    }

    return current;
}

// Получить путь к папке данных
std::string getDataPath() {
    fs::path root = findProjectRoot();
    fs::path dataPath = root / DATA_DIR;

    try {
        if (!fs::exists(dataPath)) {
            fs::create_directories(dataPath);
        }
    } catch (const std::exception&) {
        // Игнорируем ошибки доступа к файловой системе
    }

    return dataPath.string();
}

std::string normalizeJsonFilename(const std::string& filename) {
    if (filename.empty()) {
        return filename;
    }

    fs::path path(filename);
    if (path.extension().empty()) {
        path.replace_extension(".json");
    }
    return path.string();
}

std::string resolveDataFilePath(const std::string& filename) {
    if (filename.empty()) {
        return filename;
    }

    try {
        fs::path path(filename);
        if (fs::exists(path)) {
            return path.string();
        }
        if (path.is_relative()) {
            fs::path candidate = fs::path(getDataPath()) / path;
            if (fs::exists(candidate)) {
                return candidate.string();
            }
        }
    } catch (const std::exception&) {
        return filename;
    }

    return filename;
}

// Получить список JSON файлов из папки data
std::vector<std::pair<std::string, size_t>> getDataFiles() {
    std::vector<std::pair<std::string, size_t>> files;
    std::string dataPath = getDataPath();

    try {
        if (fs::exists(dataPath) && fs::is_directory(dataPath)) {
            for (const auto& entry : fs::directory_iterator(dataPath)) {
                if (entry.is_regular_file()) {
                    std::string filename = entry.path().filename().string();
                    std::string ext = entry.path().extension().string();

                    // Пропускаем служебные файлы macOS (._*)
                    if (filename.size() >= 2 && filename[0] == '.' && filename[1] == '_') {
                        continue;
                    }

                    if (ext.empty() || ext == ".json" || ext == ".JSON") {
                        files.emplace_back(filename, entry.file_size());
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        // Игнорируем ошибки доступа к файловой системе
    }

    // Сортируем по имени
    std::sort(files.begin(), files.end());
    return files;
}

// Прототипы функций
void clearScreen();
void printHeader();
void printMenu();
void printSeparator();
void pressEnterToContinue();
int getMenuChoice();
std::string getInput(const std::string& prompt);
std::string trimAscii(const std::string& input);
StreamParseResult parseStreamFile(const std::string& filename);
TolerantParseResult parseTolerantFile(const std::string& filename);

void loadFile();
void displayTree(const JsonValue& value, const std::string& prefix = "", bool isLast = true, int depth = 0, int maxDepth = 3, size_t maxItems = 20);
void showStructure();
void searchByPath();
void editValue();
void addElement();
void removeElement();
void saveFile();
void validateJson();
void showStatistics();
void showPerformanceMetrics();
void generateJsonFile();
void validateWithErrors();
void showSystemInfo();
void parallelValidation();
void generateLargeFile();
int calculateDepth(const JsonValue& value, int currentDepth = 0);

// Очистка экрана (кроссплатформенно)
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void printSeparator() {
    std::cout << "══════════════════════════════════════════════════════════════\n";
}

void printHeader() {
    clearScreen();
    printSeparator();
    std::cout << "               JSON PARSER - Курсовая работа                  \n";
    std::cout << "              Парсер JSON на C++17 (std::variant)             \n";
    printSeparator();

    if (!g_currentFile.empty()) {
        std::cout << " Файл: " << g_currentFile;
        if (g_isStreamMode) {
            std::cout << " [ПОТОКОВЫЙ РЕЖИМ]";
        } else if (g_isModified) {
            std::cout << " [изменён]";
        }
        std::cout << "\n";
        printSeparator();
    }
}

void printMenu() {
    std::cout << "\n";
    std::cout << "  [1]  Загрузить JSON файл\n";
    std::cout << "  [2]  Показать структуру (дерево)\n";
    std::cout << "  [3]  Поиск по пути (например: user.address.city)\n";
    std::cout << "  [4]  Редактировать значение\n";
    std::cout << "  [5]  Добавить элемент\n";
    std::cout << "  [6]  Удалить элемент\n";
    std::cout << "  [7]  Сохранить в файл\n";
    std::cout << "  [8]  Валидация JSON\n";
    std::cout << "  [9]  Статистика\n";
    std::cout << "  [10] Метрики выполнения\n";
    std::cout << "  [11] Генератор JSON файлов\n";
    std::cout << "  [12] Валидация с подсчётом ошибок\n";
    std::cout << "  [13] Информация о системе\n";
    std::cout << "  [14] Многопоточная валидация\n";
    std::cout << "  [15] Генератор больших файлов (ГБ)\n";
    std::cout << "  [0]  Выход\n";
    std::cout << "\n";
}

void pressEnterToContinue() {
    std::cout << "\nНажмите Enter для продолжения...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

int getMenuChoice() {
    std::cout << "Выберите пункт меню: ";
    int choice;
    std::cin >> choice;

    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return -1;
    }

    return choice;
}

std::string getInput(const std::string& prompt) {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);
    return input;
}

// Версия getInput для использования после std::cin >>
std::string getInputAfterCin(const std::string& prompt) {
    std::cout << prompt;
    std::string input;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::getline(std::cin, input);
    return input;
}

std::string trimAscii(const std::string& input) {
    size_t start = 0;
    while (start < input.size() &&
           std::isspace(static_cast<unsigned char>(input[start]))) {
        start++;
    }

    size_t end = input.size();
    while (end > start &&
           std::isspace(static_cast<unsigned char>(input[end - 1]))) {
        end--;
    }

    return input.substr(start, end - start);
}

StreamParseResult parseStreamFile(const std::string& filename) {
    StreamParseResult result;
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        result.success = false;
        result.errorMessage = "Не удалось открыть файл: " + filename;
        return result;
    }

    // Получаем размер файла для прогресс-бара
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    ProgressBar progressBar(fileSize, "Валидация");

    std::string line;
    size_t fileLine = 0;
    size_t bytesRead = 0;

    while (std::getline(file, line)) {
        fileLine++;
        result.lineCount++;
        bytesRead += line.size() + 1; // +1 для \n

        // Обновляем прогресс-бар каждые 1000 строк
        if (fileLine % 1000 == 0) {
            progressBar.update(bytesRead);
        }

        std::string trimmed = trimAscii(line);
        if (trimmed.empty()) {
            continue;
        }

        if (!trimmed.empty() && trimmed.back() == ',') {
            trimmed.pop_back();
            trimmed = trimAscii(trimmed);
        }

        if (trimmed == "[" || trimmed == "]" || trimmed == "{" || trimmed == "}") {
            continue;
        }

        if (trimmed.empty()) {
            continue;
        }

        try {
            Lexer lexer(trimmed);
            auto tokens = lexer.tokenize();
            result.tokenCount += tokens.size();

            Parser parser(std::move(tokens));
            JsonValue value = parser.parse();

            result.valueCount++;
            int depth = calculateDepth(value);
            if (depth > result.maxDepth) {
                result.maxDepth = depth;
            }
        } catch (const LexerException& e) {
            // Игнорируем ошибки - просто продолжаем валидацию
            result.success = false;
            result.errorMessage = e.what();
            result.errorLine = fileLine;
            // Не возвращаемся - продолжаем проверку
        } catch (const ParserException& e) {
            result.success = false;
            result.errorMessage = e.what();
            result.errorLine = fileLine;
            // Не возвращаемся - продолжаем проверку
        } catch (const std::exception& e) {
            result.success = false;
            result.errorMessage = e.what();
            result.errorLine = fileLine;
            // Не возвращаемся - продолжаем проверку
        }
    }

    progressBar.finish();

    // Если нашли ошибки, но обработали весь файл - считаем успехом
    if (!result.success && result.valueCount > 0) {
        result.success = true; // Частичный успех
    }

    return result;
}

// Толерантная загрузка файла - пропускает ошибочные строки и загружает валидные элементы
TolerantParseResult parseTolerantFile(const std::string& filename) {
    TolerantParseResult result;
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        result.errors.push_back("Строка 0: Не удалось открыть файл");
        return result;
    }

    // Получаем размер файла для прогресс-бара
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    ProgressBar progressBar(fileSize, "Загрузка");

    std::string line;
    size_t fileLine = 0;
    size_t bytesRead = 0;

    while (std::getline(file, line)) {
        fileLine++;
        result.totalLines++;
        bytesRead += line.size() + 1;

        // Обновляем прогресс-бар каждые 1000 строк
        if (fileLine % 1000 == 0) {
            progressBar.update(bytesRead);
        }

        std::string trimmed = trimAscii(line);
        if (trimmed.empty()) {
            continue;
        }

        // Убираем trailing comma
        if (!trimmed.empty() && trimmed.back() == ',') {
            trimmed.pop_back();
            trimmed = trimAscii(trimmed);
        }

        // Пропускаем структурные символы
        if (trimmed == "[" || trimmed == "]" || trimmed == "{" || trimmed == "}") {
            continue;
        }

        if (trimmed.empty()) {
            continue;
        }

        try {
            Lexer lexer(trimmed);
            auto tokens = lexer.tokenize();

            Parser parser(std::move(tokens));
            JsonValue value = parser.parse();

            // Успешно спарсили - добавляем в массив
            result.validElements.push_back(std::move(value));
            result.successCount++;

            int depth = calculateDepth(result.validElements.back());
            if (depth > result.maxDepth) {
                result.maxDepth = depth;
            }
        } catch (const LexerException& e) {
            // Записываем ошибку и продолжаем
            result.errorCount++;
            std::string errorMsg = "Строка " + std::to_string(fileLine) + " [LEXER]: " + e.what();
            result.errors.push_back(errorMsg);
        } catch (const ParserException& e) {
            result.errorCount++;
            std::string errorMsg = "Строка " + std::to_string(fileLine) + " [PARSER]: " + e.what();
            result.errors.push_back(errorMsg);
        } catch (const std::exception& e) {
            result.errorCount++;
            std::string errorMsg = "Строка " + std::to_string(fileLine) + " [ERROR]: " + e.what();
            result.errors.push_back(errorMsg);
        }
    }

    progressBar.finish();

    return result;
}

// Вычисление максимальной глубины вложенности
int calculateDepth(const JsonValue& value, int currentDepth) {
    int maxDepth = currentDepth;

    if (value.isObject()) {
        for (const auto& [_, v] : value.asObject()) {
            int childDepth = calculateDepth(v, currentDepth + 1);
            if (childDepth > maxDepth) maxDepth = childDepth;
        }
    } else if (value.isArray()) {
        for (const auto& v : value.asArray()) {
            int childDepth = calculateDepth(v, currentDepth + 1);
            if (childDepth > maxDepth) maxDepth = childDepth;
        }
    }

    return maxDepth;
}

// Форматирование размера файла
std::string formatFileSizeShort(size_t bytes) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);
    if (bytes >= 1024ULL * 1024 * 1024) {
        oss << (bytes / (1024.0 * 1024 * 1024)) << " ГБ";
    } else if (bytes >= 1024 * 1024) {
        oss << (bytes / (1024.0 * 1024)) << " МБ";
    } else if (bytes >= 1024) {
        oss << (bytes / 1024.0) << " КБ";
    } else {
        oss << bytes << " Б";
    }
    return oss.str();
}

// Функция загрузки файла
void loadFile() {
    printHeader();
    std::cout << "\n=== Загрузка JSON файла ===\n\n";

    // Получаем список файлов из папки data
    auto dataFiles = getDataFiles();

    std::string filename;

    if (!dataFiles.empty()) {
        std::cout << "Доступные файлы в папке data/:\n";
        printSeparator();

        for (size_t i = 0; i < dataFiles.size(); ++i) {
            std::cout << "  [" << (i + 1) << "] " << std::left << std::setw(35)
                      << dataFiles[i].first << " (" << formatFileSizeShort(dataFiles[i].second) << ")\n";
        }
        std::cout << "  [0] Указать путь вручную\n";
        printSeparator();

        std::cout << "\nВыберите файл: ";
        int choice;
        std::cin >> choice;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            choice = -1;
        }

        if (choice > 0 && choice <= static_cast<int>(dataFiles.size())) {
            filename = (fs::path(getDataPath()) / dataFiles[choice - 1].first).string();
        } else if (choice == 0) {
            filename = getInputAfterCin("Введите путь к файлу: ");
        } else {
            std::cout << "\n[!] Неверный выбор.\n";
            pressEnterToContinue();
            return;
        }
    } else {
        std::cout << "Папка data/ пуста. Используйте генератор (пункт 11 или 15) для создания файлов.\n\n";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        filename = getInput("Введите путь к файлу: ");
    }

    filename = resolveDataFilePath(filename);

    if (filename.empty()) {
        std::cout << "\n[!] Путь не указан.\n";
        pressEnterToContinue();
        return;
    }

    try {
        // Получаем размер файла
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            throw JsonException("Не удалось открыть файл: " + filename);
        }
        g_metrics.fileSize = file.tellg();
        file.close();

        std::cout << "\nРазмер файла: " << formatFileSizeShort(g_metrics.fileSize) << "\n";

        // КРИТИЧЕСКИЙ ПОРОГ: файлы > 500 МБ - только валидация без загрузки
        const size_t CRITICAL_SIZE = 500ULL * 1024 * 1024; // 500 МБ

        if (g_metrics.fileSize > CRITICAL_SIZE) {
            std::cout << "\n[!] ВНИМАНИЕ: Файл очень большой!\n\n";
            std::cout << "Для файлов такого размера доступны режимы:\n";
            std::cout << "  [1] Потоковая валидация (без загрузки в память)\n";
            std::cout << "  [2] Попытка загрузить (может привести к нехватке памяти!)\n";
            std::cout << "  [3] Отмена\n";
            std::cout << "\nРекомендуется: [1]\n";
            std::cout << "Выбор: ";

            int choice;
            std::cin >> choice;

            if (choice == 1) {
                // Потоковая валидация
                std::cout << "\nВыполняется потоковая валидация...\n\n";
                auto startTime = std::chrono::high_resolution_clock::now();
                StreamParseResult streamResult = parseStreamFile(filename);
                auto endTime = std::chrono::high_resolution_clock::now();
                double parseTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

                if (streamResult.success) {
                    std::cout << "\n";
                    printSeparator();
                    std::cout << "[OK] Файл прошел валидацию!\n";
                    printSeparator();
                    std::cout << "Элементов обработано: " << streamResult.valueCount << "\n";
                    std::cout << "Строк в файле: " << streamResult.lineCount << "\n";
                    std::cout << "Токенов: " << streamResult.tokenCount << "\n";
                    std::cout << "Макс. глубина: " << streamResult.maxDepth << "\n";
                    std::cout << "Время обработки: " << std::fixed << std::setprecision(2)
                              << (parseTimeMs / 1000.0) << " сек\n";
                    printSeparator();

                    // Сохраняем информацию о файле
                    g_currentFile = filename;
                    g_isStreamMode = true;
                    g_metrics.maxDepth = streamResult.maxDepth;
                    g_metrics.tokenCount = streamResult.tokenCount;

                    std::cout << "\n[i] Файл успешно проверен в потоковом режиме.\n";
                    std::cout << "    Доступны функции: Статистика, Метрики, Поиск по строкам.\n";
                    std::cout << "    Функции редактирования НЕ доступны (файл не загружен).\n";
                } else {
                    std::cout << "\n[ОШИБКА] " << streamResult.errorMessage;
                    if (streamResult.errorLine > 0) {
                        std::cout << " (строка " << streamResult.errorLine << ")";
                    }
                    std::cout << "\n";
                }

                pressEnterToContinue();
                return;
            } else if (choice == 3) {
                pressEnterToContinue();
                return;
            }
            // choice == 2: продолжаем загрузку (опасно!)
            std::cout << "\n[!] Попытка загрузить " << formatFileSizeShort(g_metrics.fileSize) << " в память...\n";
            std::cout << "    РЕЖИМ: Толерантная загрузка (пропускает ошибочные строки)\n";
        }

        std::cout << "\nЗагрузка файла...\n\n";

        // Измеряем время парсинга
        auto startTime = std::chrono::high_resolution_clock::now();

        // Используем толерантную загрузку для больших файлов
        TolerantParseResult tolerantResult = parseTolerantFile(filename);

        auto endTime = std::chrono::high_resolution_clock::now();
        g_metrics.parseTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

        // Создаем JSON из валидных элементов
        g_currentJson = JsonValue(std::move(tolerantResult.validElements));

        // Вычисляем глубину
        std::cout << "\nАнализ структуры...\n";
        g_metrics.maxDepth = tolerantResult.maxDepth;

        // Подсчитываем токены (приблизительно)
        g_metrics.tokenCount = g_metrics.fileSize / 10; // Примерная оценка

        g_currentFile = filename;
        g_isModified = false;

        std::cout << "\n";
        printSeparator();
        std::cout << "[OK] Файл загружен!\n";
        printSeparator();
        std::cout << "Успешно загружено элементов: " << tolerantResult.successCount << "\n";
        std::cout << "Ошибок при парсинге: " << tolerantResult.errorCount << "\n";
        std::cout << "Всего строк обработано: " << tolerantResult.totalLines << "\n";
        std::cout << "Время загрузки: " << std::fixed << std::setprecision(3) << (g_metrics.parseTimeMs / 1000.0) << " сек\n";
        std::cout << "Максимальная глубина: " << g_metrics.maxDepth << "\n";
        printSeparator();

        // Показываем первые ошибки (максимум 10)
        if (tolerantResult.errorCount > 0) {
            std::cout << "\n[!] ОБНАРУЖЕНЫ ОШИБКИ:\n\n";
            size_t showErrors = std::min(size_t(10), tolerantResult.errors.size());
            for (size_t i = 0; i < showErrors; ++i) {
                std::cout << "  " << tolerantResult.errors[i] << "\n";
            }
            if (tolerantResult.errors.size() > 10) {
                std::cout << "  ... и ещё " << (tolerantResult.errors.size() - 10) << " ошибок\n";
            }
            std::cout << "\n[i] Ошибочные строки пропущены, валидные элементы загружены.\n";
        }

    } catch (const LexerException& e) {
        std::cout << "\n[ОШИБКА ЛЕКСЕРА] " << e.what() << "\n";
    } catch (const ParserException& e) {
        std::cout << "\n[ОШИБКА ПАРСЕРА] " << e.what() << "\n";
    } catch (const JsonException& e) {
        std::cout << "\n[ОШИБКА] " << e.what() << "\n";
    } catch (const std::exception& e) {
        std::cout << "\n[ОШИБКА] " << e.what() << "\n";
    }

    pressEnterToContinue();
}

// Рекурсивный вывод дерева с ОГРАНИЧЕНИЯМИ (КРИТИЧНО для больших файлов!)
void displayTree(const JsonValue& value, const std::string& prefix, bool isLast, int depth, int maxDepth, size_t maxItems) {
    // ОГРАНИЧЕНИЕ ПО ГЛУБИНЕ - предотвращает бесконечный вывод
    if (depth >= maxDepth) {
        std::cout << prefix << "... (достигнут лимит глубины " << maxDepth << ")\n";
        return;
    }

    if (value.isObject()) {
        const auto& obj = value.asObject();

        // ОГРАНИЧЕНИЕ ПО КОЛИЧЕСТВУ КЛЮЧЕЙ
        size_t limit = std::min(maxItems, obj.size());
        bool truncated = obj.size() > maxItems;

        size_t count = 0;
        for (const auto& [key, val] : obj) {
            if (count >= limit) break;

            bool last = (count == limit - 1) && !truncated;

            std::cout << prefix << (count == 0 && depth == 0 ? "" : (last ? "└── " : "├── "));
            std::cout << "\"" << key << "\": ";

            if (val.isObject()) {
                std::cout << "{" << val.size() << " ключей}\n";
                if (depth + 1 < maxDepth) {
                    displayTree(val, prefix + (last ? "    " : "│   "), true, depth + 1, maxDepth, maxItems);
                }
            } else if (val.isArray()) {
                std::cout << "[" << val.size() << " элементов]\n";
                if (depth + 1 < maxDepth) {
                    displayTree(val, prefix + (last ? "    " : "│   "), true, depth + 1, maxDepth, maxItems);
                }
            } else {
                if (val.isString()) {
                    std::string str = val.asString();
                    if (str.length() > 80) str = str.substr(0, 77) + "...";
                    std::cout << "\"" << str << "\"";
                } else if (val.isNumber()) {
                    std::cout << val.asNumber();
                } else if (val.isBool()) {
                    std::cout << (val.asBool() ? "true" : "false");
                } else if (val.isNull()) {
                    std::cout << "null";
                }
                std::cout << "\n";
            }
            count++;
        }

        if (truncated) {
            std::cout << prefix << "└── ... (ещё " << (obj.size() - limit) << " ключей)\n";
        }
    } else if (value.isArray()) {
        const auto& arr = value.asArray();

        // ОГРАНИЧЕНИЕ ПО КОЛИЧЕСТВУ ЭЛЕМЕНТОВ МАССИВА
        size_t limit = std::min(maxItems, arr.size());
        bool truncated = arr.size() > maxItems;

        for (size_t i = 0; i < limit; ++i) {
            bool last = (i == limit - 1) && !truncated;
            const JsonValue& val = arr[i];

            std::cout << prefix << (last ? "└── " : "├── ");
            std::cout << "[" << i << "]: ";

            if (val.isObject()) {
                std::cout << "{" << val.size() << " ключей}\n";
                if (depth + 1 < maxDepth) {
                    displayTree(val, prefix + (last ? "    " : "│   "), true, depth + 1, maxDepth, maxItems);
                }
            } else if (val.isArray()) {
                std::cout << "[" << val.size() << " элементов]\n";
                if (depth + 1 < maxDepth) {
                    displayTree(val, prefix + (last ? "    " : "│   "), true, depth + 1, maxDepth, maxItems);
                }
            } else {
                if (val.isString()) {
                    std::string str = val.asString();
                    if (str.length() > 80) str = str.substr(0, 77) + "...";
                    std::cout << "\"" << str << "\"";
                } else if (val.isNumber()) {
                    std::cout << val.asNumber();
                } else if (val.isBool()) {
                    std::cout << (val.asBool() ? "true" : "false");
                } else if (val.isNull()) {
                    std::cout << "null";
                }
                std::cout << "\n";
            }
        }

        if (truncated) {
            std::cout << prefix << "└── ... (ещё " << (arr.size() - limit) << " элементов)\n";
        }
    }
}

void showStructure() {
    printHeader();
    std::cout << "\n=== Структура JSON (дерево) ===\n\n";

    if (g_currentFile.empty()) {
        std::cout << "[!] Сначала загрузите JSON файл.\n";
        pressEnterToContinue();
        return;
    }

    // Проверка потокового режима
    if (g_isStreamMode) {
        std::cout << "[!] Файл был обработан в потоковом режиме (не загружен в память).\n\n";
        std::cout << "Для просмотра структуры нужно загрузить файл полностью.\n";
        std::cout << "Это может занять много памяти (" << formatFileSizeShort(g_metrics.fileSize) << ").\n\n";
        std::cout << "Хотите попробовать загрузить файл? (да/нет): ";

        std::string answer;
        std::cin >> answer;

        if (answer != "да" && answer != "yes" && answer != "y") {
            std::cout << "\n[i] Используйте пункт [1] для повторной загрузки с выбором режима.\n";
            pressEnterToContinue();
            return;
        }

        // Пытаемся загрузить файл
        std::cout << "\n[!] Попытка загрузки " << formatFileSizeShort(g_metrics.fileSize) << " в память...\n";
        std::cout << "Это может занять несколько минут и много памяти!\n\n";

        try {
            ProgressBar progressBar(g_metrics.fileSize, "Загрузка");

            g_currentJson = Parser::parseFileWithProgress(g_currentFile, [&](size_t current, size_t total) {
                progressBar.update(current);
            });

            progressBar.finish();

            g_isStreamMode = false; // Теперь файл загружен

            std::cout << "\n[OK] Файл загружен в память!\n";
            std::cout << "Теперь можно просматривать структуру.\n\n";

            // Продолжаем показ структуры
        } catch (const std::exception& e) {
            std::cout << "\n[ОШИБКА] Не удалось загрузить файл: " << e.what() << "\n";
            std::cout << "Файл слишком большой для полной загрузки.\n";
            pressEnterToContinue();
            return;
        }
    }

    // Для больших файлов предлагаем варианты
    if (g_metrics.fileSize > 10 * 1024 * 1024) { // > 10 МБ
        std::cout << "Файл большой (" << formatFileSizeShort(g_metrics.fileSize) << ").\n\n";
        std::cout << "Варианты отображения:\n";
        std::cout << "  [1] Первые 3 уровня вложенности (быстро)\n";
        std::cout << "  [2] Первые 5 уровней вложенности (средне)\n";
        std::cout << "  [3] Полная структура (может быть медленно!)\n";
        std::cout << "  [4] Только корневой уровень\n";
        std::cout << "\nВыбор: ";

        int choice;
        std::cin >> choice;

        std::cout << "\nФормирование структуры...\n\n";

        if (choice == 1) {
            displayTree(g_currentJson, "", true, 0, 3, 20);  // 3 уровня, 20 элементов
            std::cout << "\n[Показаны первые 3 уровня, макс. 20 элементов на уровень]\n";
        } else if (choice == 2) {
            displayTree(g_currentJson, "", true, 0, 5, 15);  // 5 уровней, 15 элементов
            std::cout << "\n[Показаны первые 5 уровней, макс. 15 элементов на уровень]\n";
        } else if (choice == 4) {
            // Только корневой уровень
            if (g_currentJson.isObject()) {
                const auto& obj = g_currentJson.asObject();
                std::cout << "Объект с ключами:\n";
                for (const auto& [key, val] : obj) {
                    std::cout << "  • " << key << " : " << val.typeName() << "\n";
                }
            } else if (g_currentJson.isArray()) {
                std::cout << "Массив [" << g_currentJson.size() << " элементов]\n";
                std::cout << "Типы элементов:\n";
                size_t limit = std::min(size_t(10), g_currentJson.size());
                for (size_t i = 0; i < limit; ++i) {
                    std::cout << "  [" << i << "] : " << g_currentJson[i].typeName() << "\n";
                }
                if (g_currentJson.size() > 10) {
                    std::cout << "  ... и ещё " << (g_currentJson.size() - 10) << " элементов\n";
                }
            } else {
                std::cout << "Корневой тип: " << g_currentJson.typeName() << "\n";
            }
        } else {
            // "Полная" структура - всё равно ограничиваем!
            std::cout << "[!] ВНИМАНИЕ: Для файлов > 10 МБ показываем ограниченную структуру\n";
            std::cout << "    (макс. 10 уровней, 50 элементов на уровень)\n\n";

            displayTree(g_currentJson, "", true, 0, 10, 50);  // 10 уровней, 50 элементов
            std::cout << "\n[Показано: макс. 10 уровней вложенности, 50 элементов на уровень]\n";
        }
    } else {
        // Для маленьких файлов показываем дерево (ограниченное)
        displayTree(g_currentJson, "", true, 0, 20, 100);  // 20 уровней, 100 элементов
    }

    pressEnterToContinue();
}

void searchByPath() {
    printHeader();
    std::cout << "\n=== Поиск по пути ===\n\n";

    if (g_currentFile.empty()) {
        std::cout << "[!] Сначала загрузите JSON файл.\n";
        pressEnterToContinue();
        return;
    }

    if (g_isStreamMode) {
        std::cout << "[!] Файл в потоковом режиме. Поиск по пути недоступен.\n";
        std::cout << "    Используйте пункт [1] для загрузки файла с выбором режима [2].\n";
        pressEnterToContinue();
        return;
    }

    std::cout << "Примеры путей:\n";
    std::cout << "  - user.name\n";
    std::cout << "  - items[0].title\n";
    std::cout << "  - data.users[2].address.city\n\n";

    std::string path = getInput("Введите путь: ");

    if (path.empty()) {
        std::cout << "\n[!] Путь не указан.\n";
        pressEnterToContinue();
        return;
    }

    // Измеряем время поиска
    auto startTime = std::chrono::high_resolution_clock::now();
    auto result = g_currentJson.findByPath(path);
    auto endTime = std::chrono::high_resolution_clock::now();
    g_metrics.searchTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    if (result.has_value()) {
        const JsonValue& found = result->get();
        std::cout << "\n[OK] Найдено! Тип: " << found.typeName() << "\n";
        std::cout << "Время поиска: " << std::fixed << std::setprecision(3) << g_metrics.searchTimeMs << " мс\n";
        std::cout << "Значение:\n";
        std::cout << Serializer::toString(found, true) << "\n";
    } else {
        std::cout << "\n[!] Путь не найден: " << path << "\n";
        std::cout << "Время поиска: " << std::fixed << std::setprecision(3) << g_metrics.searchTimeMs << " мс\n";
    }

    pressEnterToContinue();
}

void editValue() {
    printHeader();
    std::cout << "\n=== Редактирование значения ===\n\n";

    if (g_currentFile.empty()) {
        std::cout << "[!] Сначала загрузите JSON файл.\n";
        pressEnterToContinue();
        return;
    }

    if (g_isStreamMode) {
        std::cout << "[!] Файл в потоковом режиме. Редактирование недоступно.\n";
        pressEnterToContinue();
        return;
    }

    std::string path = getInput("Введите путь к элементу: ");

    auto result = g_currentJson.findByPath(path);

    if (!result.has_value()) {
        std::cout << "\n[!] Путь не найден: " << path << "\n";
        pressEnterToContinue();
        return;
    }

    JsonValue& target = result->get();
    std::cout << "\nТекущее значение (" << target.typeName() << "): ";
    std::cout << Serializer::toString(target, false) << "\n\n";

    std::cout << "Выберите тип нового значения:\n";
    std::cout << "  [1] Строка\n";
    std::cout << "  [2] Число\n";
    std::cout << "  [3] Булево (true/false)\n";
    std::cout << "  [4] Null\n";
    std::cout << "  [5] JSON (ввести как строку)\n";
    std::cout << "\nВыбор: ";

    int typeChoice;
    std::cin >> typeChoice;

    std::string input;

    switch (typeChoice) {
        case 1:
            input = getInput("Введите строковое значение: ");
            target = JsonValue(input);
            g_isModified = true;
            std::cout << "\n[OK] Значение изменено.\n";
            break;

        case 2: {
            std::cout << "Введите числовое значение: ";
            double num;
            std::cin >> num;
            target = JsonValue(num);
            g_isModified = true;
            std::cout << "\n[OK] Значение изменено.\n";
            break;
        }

        case 3:
            input = getInput("Введите true или false: ");
            target = JsonValue(input == "true");
            g_isModified = true;
            std::cout << "\n[OK] Значение изменено.\n";
            break;

        case 4:
            target = JsonValue(nullptr);
            g_isModified = true;
            std::cout << "\n[OK] Значение изменено на null.\n";
            break;

        case 5:
            input = getInput("Введите JSON: ");
            try {
                target = Parser::parseString(input);
                g_isModified = true;
                std::cout << "\n[OK] Значение изменено.\n";
            } catch (const std::exception& e) {
                std::cout << "\n[ОШИБКА] Некорректный JSON: " << e.what() << "\n";
            }
            break;

        default:
            std::cout << "\n[!] Неверный выбор.\n";
            break;
    }

    pressEnterToContinue();
}

void addElement() {
    printHeader();
    std::cout << "\n=== Добавление элемента ===\n\n";

    if (g_currentFile.empty()) {
        std::cout << "[!] Сначала загрузите JSON файл.\n";
        pressEnterToContinue();
        return;
    }

    if (g_isStreamMode) {
        std::cout << "[!] Файл в потоковом режиме. Редактирование недоступно.\n";
        pressEnterToContinue();
        return;
    }

    std::string path = getInput("Введите путь к родительскому объекту/массиву (или пусто для корня): ");

    JsonValue* target = &g_currentJson;
    if (!path.empty()) {
        auto result = g_currentJson.findByPath(path);
        if (!result.has_value()) {
            std::cout << "\n[!] Путь не найден: " << path << "\n";
            pressEnterToContinue();
            return;
        }
        target = &(result->get());
    }

    if (target->isObject()) {
        std::string key = getInput("Введите ключ нового элемента: ");
        std::string value = getInput("Введите значение (JSON): ");

        try {
            (*target)[key] = Parser::parseString(value);
            g_isModified = true;
            std::cout << "\n[OK] Элемент добавлен.\n";
        } catch (const std::exception& e) {
            std::cout << "\n[ОШИБКА] Некорректный JSON: " << e.what() << "\n";
        }
    } else if (target->isArray()) {
        std::string value = getInput("Введите значение для добавления (JSON): ");

        try {
            target->push_back(Parser::parseString(value));
            g_isModified = true;
            std::cout << "\n[OK] Элемент добавлен в массив.\n";
        } catch (const std::exception& e) {
            std::cout << "\n[ОШИБКА] Некорректный JSON: " << e.what() << "\n";
        }
    } else {
        std::cout << "\n[!] Элемент по указанному пути не является объектом или массивом.\n";
    }

    pressEnterToContinue();
}

void removeElement() {
    printHeader();
    std::cout << "\n=== Удаление элемента ===\n\n";

    if (g_currentFile.empty()) {
        std::cout << "[!] Сначала загрузите JSON файл.\n";
        pressEnterToContinue();
        return;
    }

    if (g_isStreamMode) {
        std::cout << "[!] Файл в потоковом режиме. Редактирование недоступно.\n";
        pressEnterToContinue();
        return;
    }

    std::string path = getInput("Введите путь к родительскому объекту/массиву: ");

    JsonValue* target = &g_currentJson;
    if (!path.empty()) {
        auto result = g_currentJson.findByPath(path);
        if (!result.has_value()) {
            std::cout << "\n[!] Путь не найден: " << path << "\n";
            pressEnterToContinue();
            return;
        }
        target = &(result->get());
    }

    if (target->isObject()) {
        std::string key = getInput("Введите ключ для удаления: ");
        if (target->erase(key)) {
            g_isModified = true;
            std::cout << "\n[OK] Элемент удалён.\n";
        } else {
            std::cout << "\n[!] Ключ не найден: " << key << "\n";
        }
    } else if (target->isArray()) {
        std::cout << "Введите индекс для удаления: ";
        size_t index;
        std::cin >> index;

        try {
            target->erase(index);
            g_isModified = true;
            std::cout << "\n[OK] Элемент удалён.\n";
        } catch (const JsonException& e) {
            std::cout << "\n[!] " << e.what() << "\n";
        }
    } else {
        std::cout << "\n[!] Элемент по указанному пути не является объектом или массивом.\n";
    }

    pressEnterToContinue();
}

void saveFile() {
    printHeader();
    std::cout << "\n=== Сохранение в файл ===\n\n";

    if (g_currentFile.empty()) {
        std::cout << "[!] Нет данных для сохранения. Сначала загрузите JSON файл.\n";
        pressEnterToContinue();
        return;
    }

    if (g_isStreamMode) {
        std::cout << "[!] Файл в потоковом режиме. Сохранение недоступно.\n";
        std::cout << "    Используйте пункт [1] для загрузки файла с выбором режима [2].\n";
        pressEnterToContinue();
        return;
    }

    std::cout << "Текущий файл: " << g_currentFile << "\n\n";
    std::string filename = getInput("Введите путь для сохранения (Enter для перезаписи текущего): ");

    if (filename.empty()) {
        filename = g_currentFile;
    }

    std::cout << "\nФорматирование:\n";
    std::cout << "  [1] Красивый вывод (с отступами)\n";
    std::cout << "  [2] Компактный (без пробелов)\n";
    std::cout << "Выбор: ";

    int formatChoice;
    std::cin >> formatChoice;

    bool pretty = (formatChoice != 2);

    std::cout << "\nСохранение файла...\n\n";

    // Прогресс-бар для больших файлов
    ProgressBar progressBar(100, "Сериализация");

    auto startTime = std::chrono::high_resolution_clock::now();

    // Симулируем прогресс
    std::thread progressThread([&]() {
        for (int i = 0; i <= 90; i += 10) {
            progressBar.update(i);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    bool success = Serializer::toFile(g_currentJson, filename, pretty);

    progressThread.join();
    progressBar.update(100);
    progressBar.finish();

    auto endTime = std::chrono::high_resolution_clock::now();
    double saveTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    if (success) {
        std::cout << "\n[OK] Файл сохранён: " << filename << "\n";
        std::cout << "Время сохранения: " << std::fixed << std::setprecision(3) << (saveTimeMs / 1000.0) << " сек\n";
        g_isModified = false;
        g_currentFile = filename;
    } else {
        std::cout << "\n[ОШИБКА] Не удалось сохранить файл.\n";
    }

    pressEnterToContinue();
}

void validateJson() {
    printHeader();
    std::cout << "\n=== Валидация JSON ===\n\n";

    std::string input = getInput("Введите JSON для проверки (или путь к файлу): ");

    if (input.empty()) {
        std::cout << "\n[!] Ввод пуст.\n";
        pressEnterToContinue();
        return;
    }

    // Проверяем, это файл или строка JSON
    std::ifstream testFile(input);
    bool isFile = testFile.good();
    testFile.close();

    try {
        JsonValue parsed;
        if (isFile) {
            parsed = Parser::parseFile(input);
            std::cout << "\n[OK] Файл содержит валидный JSON!\n";
        } else {
            parsed = Parser::parseString(input);
            std::cout << "\n[OK] Строка содержит валидный JSON!\n";
        }
        std::cout << "Тип корневого элемента: " << parsed.typeName() << "\n";
    } catch (const LexerException& e) {
        std::cout << "\n[ОШИБКА ЛЕКСЕРА] " << e.what() << "\n";
        std::cout << "JSON невалиден.\n";
    } catch (const ParserException& e) {
        std::cout << "\n[ОШИБКА ПАРСЕРА] " << e.what() << "\n";
        std::cout << "JSON невалиден.\n";
    } catch (const std::exception& e) {
        std::cout << "\n[ОШИБКА] " << e.what() << "\n";
    }

    pressEnterToContinue();
}

// Рекурсивный подсчёт статистики
void countElements(const JsonValue& value, int& objects, int& arrays,
                   int& strings, int& numbers, int& bools, int& nulls, int& totalKeys) {
    if (value.isObject()) {
        objects++;
        const auto& obj = value.asObject();
        totalKeys += obj.size();
        for (const auto& [_, v] : obj) {
            countElements(v, objects, arrays, strings, numbers, bools, nulls, totalKeys);
        }
    } else if (value.isArray()) {
        arrays++;
        for (const auto& v : value.asArray()) {
            countElements(v, objects, arrays, strings, numbers, bools, nulls, totalKeys);
        }
    } else if (value.isString()) {
        strings++;
    } else if (value.isNumber()) {
        numbers++;
    } else if (value.isBool()) {
        bools++;
    } else if (value.isNull()) {
        nulls++;
    }
}

void showStatistics() {
    printHeader();
    std::cout << "\n=== Статистика JSON ===\n\n";

    if (g_currentFile.empty()) {
        std::cout << "[!] Сначала загрузите JSON файл.\n";
        pressEnterToContinue();
        return;
    }

    if (g_isStreamMode) {
        std::cout << "[!] Файл в потоковом режиме. Статистика недоступна.\n";
        std::cout << "    Используйте пункт [1] для загрузки файла с выбором режима [2].\n";
        pressEnterToContinue();
        return;
    }

    int objects = 0, arrays = 0, strings = 0;
    int numbers = 0, bools = 0, nulls = 0, totalKeys = 0;

    countElements(g_currentJson, objects, arrays, strings, numbers, bools, nulls, totalKeys);

    std::cout << "Файл: " << g_currentFile << "\n\n";
    printSeparator();
    std::cout << std::left;
    std::cout << std::setw(25) << "Объектов:" << objects << "\n";
    std::cout << std::setw(25) << "Массивов:" << arrays << "\n";
    std::cout << std::setw(25) << "Строк:" << strings << "\n";
    std::cout << std::setw(25) << "Чисел:" << numbers << "\n";
    std::cout << std::setw(25) << "Булевых значений:" << bools << "\n";
    std::cout << std::setw(25) << "Null значений:" << nulls << "\n";
    printSeparator();
    std::cout << std::setw(25) << "Всего ключей:" << totalKeys << "\n";
    std::cout << std::setw(25) << "Всего элементов:" << (objects + arrays + strings + numbers + bools + nulls) << "\n";
    printSeparator();

    pressEnterToContinue();
}

// Форматирование размера файла
std::string formatFileSize(size_t bytes) {
    const char* units[] = {"Б", "КБ", "МБ", "ГБ"};
    int unitIndex = 0;
    double size = static_cast<double>(bytes);

    while (size >= 1024.0 && unitIndex < 3) {
        size /= 1024.0;
        unitIndex++;
    }

    std::ostringstream oss;
    if (unitIndex == 0) {
        oss << bytes << " " << units[unitIndex];
    } else {
        oss << std::fixed << std::setprecision(2) << size << " " << units[unitIndex];
    }
    return oss.str();
}

void showPerformanceMetrics() {
    printHeader();
    std::cout << "\n=== Метрики выполнения ===\n\n";

    if (g_currentFile.empty()) {
        std::cout << "[!] Сначала загрузите JSON файл.\n";
        pressEnterToContinue();
        return;
    }

    if (g_isStreamMode) {
        std::cout << "[!] Файл в потоковом режиме. Метрики недоступны.\n";
        std::cout << "    Используйте пункт [1] для загрузки файла с выбором режима [2].\n";
        pressEnterToContinue();
        return;
    }

    // Измеряем время сериализации
    auto startTime = std::chrono::high_resolution_clock::now();
    std::string serialized = Serializer::toString(g_currentJson, true);
    auto endTime = std::chrono::high_resolution_clock::now();
    g_metrics.serializeTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    // Подсчёт элементов для дополнительной статистики
    int objects = 0, arrays = 0, strings = 0;
    int numbers = 0, bools = 0, nulls = 0, totalKeys = 0;
    countElements(g_currentJson, objects, arrays, strings, numbers, bools, nulls, totalKeys);
    int totalElements = objects + arrays + strings + numbers + bools + nulls;

    std::cout << "Файл: " << g_currentFile << "\n\n";

    printSeparator();
    std::cout << "                    МЕТРИКИ ВРЕМЕНИ                          \n";
    printSeparator();
    std::cout << std::left << std::fixed << std::setprecision(3);
    std::cout << std::setw(30) << "Время парсинга:" << g_metrics.parseTimeMs << " мс\n";
    std::cout << std::setw(30) << "Время сериализации:" << g_metrics.serializeTimeMs << " мс\n";
    std::cout << std::setw(30) << "Последний поиск:" << g_metrics.searchTimeMs << " мс\n";

    printSeparator();
    std::cout << "                    МЕТРИКИ ДАННЫХ                           \n";
    printSeparator();
    std::cout << std::setw(30) << "Размер файла:" << formatFileSize(g_metrics.fileSize) << "\n";
    std::cout << std::setw(30) << "Размер после сериализации:" << formatFileSize(serialized.size()) << "\n";
    std::cout << std::setw(30) << "Количество токенов:" << g_metrics.tokenCount << "\n";
    std::cout << std::setw(30) << "Всего элементов:" << totalElements << "\n";
    std::cout << std::setw(30) << "Максимальная глубина:" << g_metrics.maxDepth << "\n";

    printSeparator();
    std::cout << "                    ПРОИЗВОДИТЕЛЬНОСТЬ                       \n";
    printSeparator();

    if (g_metrics.parseTimeMs > 0) {
        double tokensPerMs = g_metrics.tokenCount / g_metrics.parseTimeMs;
        double bytesPerMs = g_metrics.fileSize / g_metrics.parseTimeMs;
        std::cout << std::setw(30) << "Токенов/мс:" << std::setprecision(1) << tokensPerMs << "\n";
        std::cout << std::setw(30) << "Байт/мс (парсинг):" << std::setprecision(1) << bytesPerMs << "\n";
    }

    if (g_metrics.serializeTimeMs > 0) {
        double bytesPerMsSerial = serialized.size() / g_metrics.serializeTimeMs;
        std::cout << std::setw(30) << "Байт/мс (сериализация):" << std::setprecision(1) << bytesPerMsSerial << "\n";
    }

    double compressionRatio = (g_metrics.fileSize > 0)
        ? (static_cast<double>(serialized.size()) / g_metrics.fileSize * 100.0)
        : 0.0;
    std::cout << std::setw(30) << "Коэфф. размера (%):" << std::setprecision(1) << compressionRatio << "%\n";

    printSeparator();

    pressEnterToContinue();
}

void generateJsonFile() {
    printHeader();
    std::cout << "\n=== Генератор JSON файлов ===\n\n";

    Generator generator;
    GeneratorOptions opts;

    std::cout << "Настройки генерации:\n\n";

    std::cout << "Максимальная глубина вложенности (1-10) [5]: ";
    int depth;
    std::cin >> depth;
    if (std::cin.fail() || depth < 1 || depth > 10) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        depth = 5;
    }
    opts.maxDepth = depth;

    std::cout << "Максимальный размер массивов (1-20) [10]: ";
    int arraySize;
    std::cin >> arraySize;
    if (std::cin.fail() || arraySize < 1 || arraySize > 20) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        arraySize = 10;
    }
    opts.maxArraySize = arraySize;

    std::cout << "Максимальное кол-во ключей в объекте (1-15) [8]: ";
    int objectKeys;
    std::cin >> objectKeys;
    if (std::cin.fail() || objectKeys < 1 || objectKeys > 15) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        objectKeys = 8;
    }
    opts.maxObjectKeys = objectKeys;

    std::cout << "\nВнести ошибки в JSON?\n";
    std::cout << "  [0] Без ошибок (валидный JSON)\n";
    std::cout << "  [1-5] Количество ошибок\n";
    std::cout << "Выбор: ";

    int errorCount;
    std::cin >> errorCount;
    if (std::cin.fail() || errorCount < 0) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        errorCount = 0;
    }
    if (errorCount > 5) errorCount = 5;

    generator.setOptions(opts);

    std::string filename = getInputAfterCin("\nИмя файла для сохранения (в data/): ");
    if (filename.empty()) {
        filename = "generated.json";
    }
    filename = normalizeJsonFilename(filename);

    // Добавляем путь к папке data
    std::string fullPath = (fs::path(getDataPath()) / filename).string();

    // Генерация
    auto startTime = std::chrono::high_resolution_clock::now();

    std::string json;
    if (errorCount > 0) {
        json = generator.generateWithErrors(errorCount);
    } else {
        json = generator.generate();
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    double genTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    // Сохранение
    if (generator.saveToFile(fullPath, json)) {
        std::cout << "\n[OK] Файл сгенерирован и сохранён: " << fullPath << "\n";
        std::cout << "Размер: " << json.size() << " байт\n";
        std::cout << "Время генерации: " << std::fixed << std::setprecision(3) << genTimeMs << " мс\n";

        if (errorCount > 0) {
            std::cout << "\n[!] В файл внесено ошибок: " << generator.getGeneratedErrors().size() << "\n";
            printSeparator();
            std::cout << "Список внесённых ошибок:\n";
            int num = 1;
            for (const auto& err : generator.getGeneratedErrors()) {
                std::cout << "  " << num++ << ". " << err.description;
                std::cout << " (примерно строка " << err.line << ")\n";
            }
        }
    } else {
        std::cout << "\n[ОШИБКА] Не удалось сохранить файл.\n";
    }

    pressEnterToContinue();
}

void validateWithErrors() {
    printHeader();
    std::cout << "\n=== Валидация с подсчётом ошибок ===\n\n";

    std::string input = getInput("Введите путь к файлу или JSON строку: ");

    if (input.empty()) {
        std::cout << "\n[!] Ввод пуст.\n";
        pressEnterToContinue();
        return;
    }

    // Определяем, файл это или строка
    std::ifstream testFile(input);
    bool isFile = testFile.good();
    testFile.close();

    auto startTime = std::chrono::high_resolution_clock::now();

    Validator validator(false); // Не останавливаться на первой ошибке
    ValidationResult result;

    if (isFile) {
        result = validator.validateFile(input);
        std::cout << "\nВалидация файла: " << input << "\n";
    } else {
        result = validator.validate(input);
        std::cout << "\nВалидация JSON строки\n";
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    double validateTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    printSeparator();
    std::cout << "                    РЕЗУЛЬТАТ ВАЛИДАЦИИ                      \n";
    printSeparator();

    std::cout << std::left;
    std::cout << std::setw(25) << "Статус:" << (result.isValid ? "ВАЛИДНЫЙ" : "НЕВАЛИДНЫЙ") << "\n";
    std::cout << std::setw(25) << "Время валидации:" << std::fixed << std::setprecision(3) << validateTimeMs << " мс\n";
    std::cout << std::setw(25) << "Строк в файле:" << result.lineCount << "\n";
    std::cout << std::setw(25) << "Токенов:" << result.tokenCount << "\n";
    std::cout << std::setw(25) << "Найдено ошибок:" << result.errors.size() << "\n";

    if (!result.errors.empty()) {
        printSeparator();
        std::cout << "                    СПИСОК ОШИБОК                             \n";
        printSeparator();

        int errorNum = 1;
        for (const auto& err : result.errors) {
            std::cout << "\n[Ошибка " << errorNum++ << "]\n";
            std::cout << "  Позиция: строка " << err.line << ", столбец " << err.column << "\n";
            std::cout << "  Сообщение: " << err.message << "\n";
            if (!err.context.empty()) {
                std::cout << "  Контекст: " << err.context << "\n";
                // Показываем указатель на позицию ошибки
                if (err.column > 0 && err.column <= err.context.size() + 10) {
                    std::cout << "            ";
                    for (size_t i = 1; i < err.column && i < 60; ++i) {
                        std::cout << " ";
                    }
                    std::cout << "^\n";
                }
            }
        }
    }

    printSeparator();

    // Если валидный, предлагаем загрузить
    if (result.isValid && isFile) {
        std::cout << "\nJSON валиден. Загрузить в редактор? (да/нет): ";
        std::string answer;
        std::cin >> answer;
        if (answer == "да" || answer == "yes" || answer == "y") {
            try {
                g_currentJson = Parser::parseFile(input);
                g_currentFile = input;
                g_isModified = false;
                std::cout << "[OK] Файл загружен!\n";
            } catch (const std::exception& e) {
                std::cout << "[ОШИБКА] " << e.what() << "\n";
            }
        }
    }

    pressEnterToContinue();
}

void showSystemInfo() {
    printHeader();
    std::cout << "\n=== Информация о системе ===\n\n";

    CPUInfo cpuInfo = SystemInfo::getCPUInfo();
    size_t availableMemMB = SystemInfo::getAvailableMemoryMB();

    printSeparator();
    std::cout << "                    ПРОЦЕССОР                                \n";
    printSeparator();

    std::cout << std::left;
    std::cout << std::setw(30) << "Модель:" << cpuInfo.name << "\n";
    std::cout << std::setw(30) << "Физических ядер:" << cpuInfo.physicalCores << "\n";
    std::cout << std::setw(30) << "Логических ядер (потоков):" << cpuInfo.logicalCores << "\n";

    printSeparator();
    std::cout << "                    ПАМЯТЬ                                   \n";
    printSeparator();

    if (availableMemMB > 0) {
        if (availableMemMB >= 1024) {
            std::cout << std::setw(30) << "Доступно RAM:"
                      << std::fixed << std::setprecision(2)
                      << (availableMemMB / 1024.0) << " ГБ\n";
        } else {
            std::cout << std::setw(30) << "Доступно RAM:" << availableMemMB << " МБ\n";
        }
    }

    printSeparator();
    std::cout << "                    РЕКОМЕНДАЦИИ                             \n";
    printSeparator();

    std::cout << std::setw(30) << "Потоков для обработки:" << "от 1 до " << cpuInfo.logicalCores << "\n";
    std::cout << std::setw(30) << "Рекомендуется:" << cpuInfo.recommendedThreads << " потоков\n";

    // Рекомендация по размеру файлов
    size_t maxFileSizeMB = availableMemMB / 4; // Используем 25% памяти
    if (maxFileSizeMB > 0) {
        std::cout << std::setw(30) << "Макс. размер файла:"
                  << (maxFileSizeMB >= 1024 ? std::to_string(maxFileSizeMB / 1024) + " ГБ"
                                            : std::to_string(maxFileSizeMB) + " МБ") << "\n";
    }

    printSeparator();

    pressEnterToContinue();
}

void parallelValidation() {
    printHeader();
    std::cout << "\n=== Многопоточная валидация ===\n\n";

    CPUInfo cpuInfo = SystemInfo::getCPUInfo();

    std::cout << "Процессор: " << cpuInfo.name << "\n";
    std::cout << "Доступно потоков: от 1 до " << cpuInfo.logicalCores << "\n";
    std::cout << "Рекомендуется: " << cpuInfo.recommendedThreads << " потоков\n\n";

    std::cout << "Количество потоков [" << cpuInfo.recommendedThreads << "]: ";
    int threadCount;
    std::cin >> threadCount;

    if (std::cin.fail() || threadCount < 1) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        threadCount = cpuInfo.recommendedThreads;
    }

    if (threadCount > static_cast<int>(cpuInfo.logicalCores)) {
        threadCount = cpuInfo.logicalCores;
        std::cout << "[!] Ограничено до " << threadCount << " потоков\n";
    }

    // Получаем список файлов из папки data
    auto dataFiles = getDataFiles();
    std::string filename;

    if (!dataFiles.empty()) {
        std::cout << "\nДоступные файлы в папке data/:\n";
        printSeparator();

        for (size_t i = 0; i < dataFiles.size(); ++i) {
            std::cout << "  [" << (i + 1) << "] " << std::left << std::setw(35)
                      << dataFiles[i].first << " (" << formatFileSizeShort(dataFiles[i].second) << ")\n";
        }
        std::cout << "  [0] Указать путь вручную\n";
        printSeparator();

        std::cout << "\nВыберите файл: ";
        int choice;
        std::cin >> choice;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            choice = -1;
        }

        if (choice > 0 && choice <= static_cast<int>(dataFiles.size())) {
            filename = (fs::path(getDataPath()) / dataFiles[choice - 1].first).string();
        } else if (choice == 0) {
            filename = getInputAfterCin("Введите путь к файлу: ");
        } else {
            std::cout << "\n[!] Неверный выбор.\n";
            pressEnterToContinue();
            return;
        }
    } else {
        filename = getInputAfterCin("\nПуть к JSON файлу: ");
    }

    if (filename.empty()) {
        std::cout << "\n[!] Путь не указан.\n";
        pressEnterToContinue();
        return;
    }

    // Проверяем размер файла
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cout << "\n[ОШИБКА] Не удалось открыть файл.\n";
        pressEnterToContinue();
        return;
    }

    size_t fileSize = file.tellg();
    file.close();

    std::cout << "\nРазмер файла: " << formatFileSize(fileSize) << "\n";
    std::cout << "Используется потоков: " << threadCount << "\n";
    std::cout << "\nВыполняется валидация...\n\n";

    ParallelProcessor processor(threadCount);

    // Прогресс-бар
    auto progressCallback = [](const ProcessingProgress& progress) {
        if (progress.totalBytes > 0) {
            int percent = (progress.processedBytes * 100) / progress.totalBytes;
            std::cout << "\r[";
            int barWidth = 40;
            int pos = (barWidth * percent) / 100;
            for (int i = 0; i < barWidth; ++i) {
                if (i < pos) std::cout << "=";
                else if (i == pos) std::cout << ">";
                else std::cout << " ";
            }
            std::cout << "] " << percent << "% (" << progress.errorsFound << " ошибок)";
            std::cout.flush();
        }
    };

    auto result = processor.validateLargeFile(filename, progressCallback);

    std::cout << "\n\n";
    printSeparator();
    std::cout << "                    РЕЗУЛЬТАТ                                \n";
    printSeparator();

    std::cout << std::left;
    std::cout << std::setw(30) << "Статус:" << (result.success ? "ВАЛИДНЫЙ" : "НЕВАЛИДНЫЙ") << "\n";
    std::cout << std::setw(30) << "Время обработки:" << std::fixed << std::setprecision(2)
              << result.totalTimeMs << " мс\n";
    std::cout << std::setw(30) << "Обработано чанков:" << result.processedChunks << " из " << result.totalChunks << "\n";
    std::cout << std::setw(30) << "Найдено ошибок:" << result.totalErrors << "\n";
    std::cout << std::setw(30) << "Пропускная способность:" << std::fixed << std::setprecision(2)
              << result.throughputMBps << " МБ/с\n";

    // Показываем первые 10 ошибок
    if (!result.errors.empty()) {
        printSeparator();
        std::cout << "Первые ошибки (макс. 10):\n\n";

        int count = 0;
        for (const auto& err : result.errors) {
            if (++count > 10) {
                std::cout << "... и ещё " << (result.errors.size() - 10) << " ошибок\n";
                break;
            }
            std::cout << "  [" << count << "] Строка " << err.line << ": " << err.message << "\n";
        }
    }

    printSeparator();

    pressEnterToContinue();
}

void generateLargeFile() {
    printHeader();
    std::cout << "\n=== Генератор больших JSON файлов ===\n\n";

    CPUInfo cpuInfo = SystemInfo::getCPUInfo();
    size_t availableMemMB = SystemInfo::getAvailableMemoryMB();

    std::cout << "Процессор: " << cpuInfo.name << "\n";
    std::cout << "Доступно потоков: " << cpuInfo.logicalCores << "\n";
    if (availableMemMB > 0) {
        std::cout << "Доступно памяти: " << (availableMemMB / 1024) << " ГБ\n";
    }
    std::cout << "\n";

    printSeparator();

    std::cout << "\nВыберите размер файла:\n";
    std::cout << "  [1] 100 МБ\n";
    std::cout << "  [2] 500 МБ\n";
    std::cout << "  [3] 1 ГБ\n";
    std::cout << "  [4] 2 ГБ\n";
    std::cout << "  [5] 5 ГБ\n";
    std::cout << "  [6] 10 ГБ\n";
    std::cout << "  [7] 15 ГБ\n";
    std::cout << "  [8] Указать вручную (в МБ)\n";
    std::cout << "Выбор: ";

    int sizeChoice;
    std::cin >> sizeChoice;

    size_t targetSizeMB = 100;
    switch (sizeChoice) {
        case 1: targetSizeMB = 100; break;
        case 2: targetSizeMB = 500; break;
        case 3: targetSizeMB = 1024; break;
        case 4: targetSizeMB = 2048; break;
        case 5: targetSizeMB = 5120; break;
        case 6: targetSizeMB = 10240; break;
        case 7: targetSizeMB = 15360; break;
        case 8:
            std::cout << "Размер в МБ: ";
            std::cin >> targetSizeMB;
            if (std::cin.fail() || targetSizeMB < 1) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                targetSizeMB = 100;
            }
            break;
        default:
            targetSizeMB = 100;
    }

    // Проверка доступной памяти
    if (availableMemMB > 0 && targetSizeMB > availableMemMB / 2) {
        std::cout << "\n[!] Внимание: размер файла (" << targetSizeMB
                  << " МБ) превышает половину доступной памяти.\n";
        std::cout << "Продолжить? (да/нет): ";
        std::string answer;
        std::cin >> answer;
        if (answer != "да" && answer != "yes" && answer != "y") {
            pressEnterToContinue();
            return;
        }
    }

    std::cout << "\nКоличество потоков (1-" << cpuInfo.logicalCores << ") ["
              << cpuInfo.recommendedThreads << "]: ";
    int threadCount;
    std::cin >> threadCount;

    if (std::cin.fail() || threadCount < 1) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        threadCount = cpuInfo.recommendedThreads;
    }

    if (threadCount > static_cast<int>(cpuInfo.logicalCores)) {
        threadCount = cpuInfo.logicalCores;
    }

    std::cout << "Глубина вложенности (1-5) [3]: ";
    int depth;
    std::cin >> depth;
    if (std::cin.fail() || depth < 1 || depth > 5) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        depth = 3;
    }

    std::cout << "Добавить ошибки? (0-100%) [0]: ";
    int errorProb;
    std::cin >> errorProb;
    if (std::cin.fail() || errorProb < 0 || errorProb > 100) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        errorProb = 0;
    }

    std::string filename = getInputAfterCin("\nИмя файла (сохраняется в data/): ");
    if (filename.empty()) {
        filename = "large_" + std::to_string(targetSizeMB) + "mb.json";
    }
    filename = normalizeJsonFilename(filename);

    // Добавляем путь к папке data
    std::string fullPath = (fs::path(getDataPath()) / filename).string();

    size_t targetSizeBytes = static_cast<size_t>(targetSizeMB) * 1024ULL * 1024ULL;

    std::cout << "\nГенерация файла...\n";
    std::cout << "Файл: " << fullPath << "\n";
    std::cout << "Целевой размер: " << targetSizeMB << " МБ\n";
    std::cout << "Потоков: " << threadCount << "\n\n";

    ParallelGenerator generator(threadCount);

    auto startTime = std::chrono::high_resolution_clock::now();

    // Прогресс-бар
    auto progressCallback = [targetSizeBytes](size_t current, size_t /*total*/) {
        int percent = (current * 100) / targetSizeBytes;
        std::cout << "\r[";
        int barWidth = 40;
        int pos = (barWidth * percent) / 100;
        for (int i = 0; i < barWidth; ++i) {
            if (i < pos) std::cout << "=";
            else if (i == pos) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] " << percent << "% (" << (current / (1024 * 1024)) << " МБ)";
        std::cout.flush();
    };

    bool success = generator.generateLargeFile(fullPath, targetSizeBytes, depth, errorProb, progressCallback);

    auto endTime = std::chrono::high_resolution_clock::now();
    double genTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    std::cout << "\n\n";

    if (success) {
        // Получаем реальный размер файла
        std::ifstream checkFile(fullPath, std::ios::binary | std::ios::ate);
        size_t actualSize = checkFile.tellg();
        checkFile.close();

        printSeparator();
        std::cout << "[OK] Файл сгенерирован: " << fullPath << "\n";
        printSeparator();

        std::cout << std::left;
        std::cout << std::setw(30) << "Размер файла:" << formatFileSize(actualSize) << "\n";
        std::cout << std::setw(30) << "Время генерации:" << std::fixed << std::setprecision(2)
                  << (genTimeMs / 1000.0) << " сек\n";
        std::cout << std::setw(30) << "Скорость:"
                  << std::fixed << std::setprecision(2)
                  << ((actualSize / (1024.0 * 1024.0)) / (genTimeMs / 1000.0)) << " МБ/с\n";

        if (errorProb > 0) {
            std::cout << std::setw(30) << "Ошибки:" << "добавлены (" << errorProb << "% вероятность)\n";
        }

        printSeparator();
    } else {
        std::cout << "[ОШИБКА] Не удалось создать файл.\n";
    }

    pressEnterToContinue();
}

int main() {
    while (true) {
        printHeader();
        printMenu();

        int choice = getMenuChoice();

        switch (choice) {
            case 1: loadFile(); break;
            case 2: showStructure(); break;
            case 3: searchByPath(); break;
            case 4: editValue(); break;
            case 5: addElement(); break;
            case 6: removeElement(); break;
            case 7: saveFile(); break;
            case 8: validateJson(); break;
            case 9: showStatistics(); break;
            case 10: showPerformanceMetrics(); break;
            case 11: generateJsonFile(); break;
            case 12: validateWithErrors(); break;
            case 13: showSystemInfo(); break;
            case 14: parallelValidation(); break;
            case 15: generateLargeFile(); break;
            case 0:
                if (g_isModified) {
                    printHeader();
                    std::cout << "\n[!] Есть несохранённые изменения!\n";
                    std::string confirm = getInput("Выйти без сохранения? (да/нет): ");
                    if (confirm != "да" && confirm != "yes" && confirm != "y") {
                        continue;
                    }
                }
                std::cout << "\nДо свидания!\n";
                return 0;
            default:
                std::cout << "\n[!] Неверный выбор. Попробуйте снова.\n";
                pressEnterToContinue();
                break;
        }
    }

    return 0;
}
