#include <gtest/gtest.h>
#include "Parser.hpp"
#include "Validator.hpp"
#include "Generator.hpp"
#include "ParallelProcessor.hpp"
#include <fstream>
#include <filesystem>
#include <chrono>

using namespace json;
namespace fs = std::filesystem;

// Класс для интеграционных тестов с большими файлами
class LargeFileTest : public ::testing::Test {
protected:
    std::string testDir;

    void SetUp() override {
        testDir = "test_data_temp";
        fs::create_directory(testDir);
    }

    void TearDown() override {
        fs::remove_all(testDir);
    }

    std::string createTestFile(const std::string& content, const std::string& filename) {
        std::string filepath = testDir + "/" + filename;
        std::ofstream file(filepath);
        file << content;
        file.close();
        return filepath;
    }

    size_t getFileSize(const std::string& filepath) {
        return fs::file_size(filepath);
    }
};

// Тест для парсинга среднего файла (1-10 MB)
TEST_F(LargeFileTest, ParseMediumFile_1MB) {
    // Генерация JSON массива с 10000 объектами
    std::string json = "[";
    for (int i = 0; i < 10000; ++i) {
        if (i > 0) json += ",";
        json += R"({"id":)" + std::to_string(i) +
                R"(,"name":"User)" + std::to_string(i) +
                R"(","email":"user)" + std::to_string(i) + R"(@example.com"})";
    }
    json += "]";

    auto filepath = createTestFile(json, "medium_1mb.json");

    auto start = std::chrono::high_resolution_clock::now();
    auto value = Parser::parseFile(filepath);
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_TRUE(value.isArray());
    EXPECT_EQ(value.size(), 10000);
    EXPECT_LT(duration.count(), 5000);  // Должно завершиться менее чем за 5 секунд

    std::cout << "Parse time for 1MB file: " << duration.count() << " ms\n";
    std::cout << "File size: " << getFileSize(filepath) / 1024.0 / 1024.0 << " MB\n";
}

// Тест для валидации среднего файла
TEST_F(LargeFileTest, ValidateMediumFile) {
    std::string json = "[";
    for (int i = 0; i < 5000; ++i) {
        if (i > 0) json += ",";
        json += std::to_string(i);
    }
    json += "]";

    auto filepath = createTestFile(json, "validate_medium.json");

    Validator validator;
    auto start = std::chrono::high_resolution_clock::now();
    auto result = validator.validateFile(filepath);
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_TRUE(result.isValid);
    EXPECT_EQ(result.errors.size(), 0);
    EXPECT_GT(result.tokenCount, 5000);

    std::cout << "Validation time: " << duration.count() << " ms\n";
    std::cout << "Token count: " << result.tokenCount << "\n";
}

// Тест для парсинга большого файла (10-100 MB)
TEST_F(LargeFileTest, ParseLargeFile_10MB) {
    // Генерация JSON массива с 100000 объектами
    std::string json = "[";
    for (int i = 0; i < 100000; ++i) {
        if (i > 0) json += ",";
        json += R"({"id":)" + std::to_string(i) +
                R"(,"value":)" + std::to_string(i * 3.14) + "}";
    }
    json += "]";

    auto filepath = createTestFile(json, "large_10mb.json");

    std::cout << "File size: " << getFileSize(filepath) / 1024.0 / 1024.0 << " MB\n";

    auto start = std::chrono::high_resolution_clock::now();
    auto value = Parser::parseFile(filepath);
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_TRUE(value.isArray());
    EXPECT_EQ(value.size(), 100000);

    std::cout << "Parse time for ~10MB file: " << duration.count() << " ms\n";
    std::cout << "Throughput: " << (getFileSize(filepath) / 1024.0 / 1024.0) / (duration.count() / 1000.0) << " MB/s\n";
}

// Тест для параллельной валидации большого файла
TEST_F(LargeFileTest, ParallelValidation_LargeFile) {
    // Генерация большого JSON массива
    std::string json = "[";
    for (int i = 0; i < 50000; ++i) {
        if (i > 0) json += ",";
        json += R"({"id":)" + std::to_string(i) +
                R"(,"name":"Item )" + std::to_string(i) +
                R"(","active":)" + (i % 2 == 0 ? "true" : "false") + "}";
    }
    json += "]";

    auto filepath = createTestFile(json, "parallel_validation.json");

    ParallelProcessor processor(4);  // 4 потока

    auto start = std::chrono::high_resolution_clock::now();
    auto result = processor.validateLargeFile(filepath);
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.totalErrors, 0);

    std::cout << "Parallel validation time: " << duration.count() << " ms\n";
    std::cout << "Throughput: " << result.throughputMBps << " MB/s\n";
    std::cout << "Chunks processed: " << result.processedChunks << "/" << result.totalChunks << "\n";
}

// Тест для парсинга вложенных структур
TEST_F(LargeFileTest, ParseDeeplyNestedStructure) {
    std::string json = R"({
        "level1": {
            "level2": {
                "level3": {
                    "level4": {
                        "level5": {
                            "data": [1, 2, 3, 4, 5],
                            "info": {
                                "name": "deep",
                                "value": 42
                            }
                        }
                    }
                }
            }
        }
    })";

    auto filepath = createTestFile(json, "nested.json");

    auto value = Parser::parseFile(filepath);

    EXPECT_TRUE(value.isObject());
    EXPECT_TRUE(value.at("level1").at("level2").at("level3")
                     .at("level4").at("level5").at("data").isArray());
    EXPECT_EQ(value.at("level1").at("level2").at("level3")
                   .at("level4").at("level5").at("data").size(), 5);
}

// Тест для обработки файлов с ошибками
TEST_F(LargeFileTest, DetectErrorsInLargeFile) {
    // JSON с ошибкой в середине
    std::string json = "[";
    for (int i = 0; i < 1000; ++i) {
        if (i > 0) json += ",";
        if (i == 500) {
            json += "invalid";  // Ошибка
        } else {
            json += std::to_string(i);
        }
    }
    json += "]";

    auto filepath = createTestFile(json, "invalid.json");

    Validator validator;
    auto result = validator.validateFile(filepath);

    EXPECT_FALSE(result.isValid);
    EXPECT_GT(result.errors.size(), 0);
}

// Тест для работы с различными кодировками
TEST_F(LargeFileTest, ParseUnicodeContent) {
    std::string json = R"({
        "russian": "Привет мир",
        "chinese": "你好世界",
        "japanese": "こんにちは世界",
        "arabic": "مرحبا بالعالم",
        "emoji": "🌍🌎🌏"
    })";

    auto filepath = createTestFile(json, "unicode.json");

    auto value = Parser::parseFile(filepath);

    EXPECT_TRUE(value.isObject());
    EXPECT_TRUE(value.contains("russian"));
    EXPECT_TRUE(value.contains("chinese"));
    EXPECT_TRUE(value.contains("emoji"));
}

// Тест производительности: сравнение размеров файлов
TEST_F(LargeFileTest, PerformanceComparison_FileSizes) {
    std::vector<int> sizes = {100, 1000, 10000};

    for (int size : sizes) {
        std::string json = "[";
        for (int i = 0; i < size; ++i) {
            if (i > 0) json += ",";
            json += std::to_string(i);
        }
        json += "]";

        auto filepath = createTestFile(json, "perf_" + std::to_string(size) + ".json");

        auto start = std::chrono::high_resolution_clock::now();
        auto value = Parser::parseFile(filepath);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::cout << "Size: " << size << " elements, Time: " << duration.count() << " μs\n";
    }
}

// Стресс-тест: парсинг множества файлов подряд
TEST_F(LargeFileTest, StressTest_MultipleFiles) {
    const int numFiles = 10;
    std::vector<std::string> files;

    // Создаем несколько файлов
    for (int f = 0; f < numFiles; ++f) {
        std::string json = "[";
        for (int i = 0; i < 1000; ++i) {
            if (i > 0) json += ",";
            json += std::to_string(i);
        }
        json += "]";

        files.push_back(createTestFile(json, "stress_" + std::to_string(f) + ".json"));
    }

    // Парсим все файлы
    auto start = std::chrono::high_resolution_clock::now();
    for (const auto& file : files) {
        auto value = Parser::parseFile(file);
        EXPECT_TRUE(value.isArray());
        EXPECT_EQ(value.size(), 1000);
    }
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Total time for " << numFiles << " files: " << duration.count() << " ms\n";
    std::cout << "Average time per file: " << duration.count() / numFiles << " ms\n";
}
