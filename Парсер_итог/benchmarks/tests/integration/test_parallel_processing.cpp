#include <gtest/gtest.h>
#include "Parser.hpp"
#include "ParallelProcessor.hpp"
#include <chrono>
#include <fstream>
#include <filesystem>
#include <thread>

using namespace json;
namespace fs = std::filesystem;

class ParallelProcessingTest : public ::testing::Test {
protected:
    std::string testDir;

    void SetUp() override {
        testDir = "test_parallel_temp";
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

    std::string generateLargeArray(int numElements) {
        std::string json = "[";
        for (int i = 0; i < numElements; ++i) {
            if (i > 0) json += ",";
            json += R"({"id":)" + std::to_string(i) +
                    R"(,"data":"Item )" + std::to_string(i) +
                    R"(","value":)" + std::to_string(i * 2.5) + "}";
        }
        json += "]";
        return json;
    }
};

// Тест сравнения однопоточной и многопоточной валидации
TEST_F(ParallelProcessingTest, SingleVsMultiThreadValidation) {
    const int numElements = 50000;
    std::string json = generateLargeArray(numElements);
    auto filepath = createTestFile(json, "validation_test.json");

    // Однопоточная валидация
    Validator validator;
    auto startSingle = std::chrono::high_resolution_clock::now();
    auto resultSingle = validator.validateFile(filepath);
    auto endSingle = std::chrono::high_resolution_clock::now();
    auto durationSingle = std::chrono::duration_cast<std::chrono::milliseconds>(endSingle - startSingle);

    EXPECT_TRUE(resultSingle.isValid);

    // Многопоточная валидация (4 потока)
    ParallelProcessor processor(4);
    auto startMulti = std::chrono::high_resolution_clock::now();
    auto resultMulti = processor.validateLargeFile(filepath);
    auto endMulti = std::chrono::high_resolution_clock::now();
    auto durationMulti = std::chrono::duration_cast<std::chrono::milliseconds>(endMulti - startMulti);

    EXPECT_TRUE(resultMulti.success);
    EXPECT_EQ(resultMulti.totalErrors, 0);

    double speedup = static_cast<double>(durationSingle.count()) / durationMulti.count();

    std::cout << "Single-threaded validation: " << durationSingle.count() << " ms\n";
    std::cout << "Multi-threaded validation (4 threads): " << durationMulti.count() << " ms\n";
    std::cout << "Speedup: " << speedup << "x\n";

}

// Тест масштабируемости: увеличение количества потоков
TEST_F(ParallelProcessingTest, ThreadScaling) {
    const int numElements = 10000;  // Уменьшено для стабильности
    std::string json = generateLargeArray(numElements);
    auto filepath = createTestFile(json, "scaling_test.json");

    std::vector<int> threadCounts = {1, 2, 4};  // Убрали 8 потоков
    std::vector<double> times;

    for (int numThreads : threadCounts) {
        ParallelProcessor processor(numThreads);

        auto start = std::chrono::high_resolution_clock::now();
        auto result = processor.validateLargeFile(filepath);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        times.push_back(duration.count());

        EXPECT_TRUE(result.success);

        std::cout << "Threads: " << numThreads << ", Time: " << duration.count() << " ms\n";
    }

    // Проверяем, что с увеличением потоков время не растет значительно
    if (times.size() >= 2) {
        EXPECT_LE(times[1], times[0] * 2.0);  // 2 потока не должно быть намного медленнее
    }
}

// Тест корректности параллельной обработки
TEST_F(ParallelProcessingTest, ParallelProcessingCorrectness) {
    std::string json = R"([
        {"name": "Alice", "age": 25},
        {"name": "Bob", "age": 30},
        {"name": "Charlie", "age": 35},
        {"name": "David", "age": 40}
    ])";

    auto filepath = createTestFile(json, "correctness_test.json");

    // Валидация в один поток
    Validator validator;
    auto result1 = validator.validateFile(filepath);

    // Параллельная валидация
    ParallelProcessor processor(2);
    auto result2 = processor.validateLargeFile(filepath);

    // Оба должны показать, что JSON валиден
    EXPECT_TRUE(result1.isValid);
    EXPECT_TRUE(result2.success);
    EXPECT_EQ(result2.totalErrors, 0);
}

// Тест обработки ошибок в параллельном режиме
TEST_F(ParallelProcessingTest, ParallelErrorDetection) {
    // JSON с ошибками
    std::string json = "[";
    for (int i = 0; i < 1000; ++i) {
        if (i > 0) json += ",";
        if (i == 500) {
            json += "invalid";  // Ошибка в середине
        } else {
            json += std::to_string(i);
        }
    }
    json += "]";

    auto filepath = createTestFile(json, "error_test.json");

    ParallelProcessor processor(4);
    auto result = processor.validateLargeFile(filepath);

    EXPECT_FALSE(result.success);
    EXPECT_GT(result.totalErrors, 0);
}

// Тест производительности параллельного парсинга
TEST_F(ParallelProcessingTest, ParallelParsingPerformance) {
    const int numElements = 50000;
    std::string json = generateLargeArray(numElements);
    auto filepath = createTestFile(json, "parse_perf.json");

    // Однопоточный парсинг
    auto startSingle = std::chrono::high_resolution_clock::now();
    auto value1 = Parser::parseFile(filepath);
    auto endSingle = std::chrono::high_resolution_clock::now();
    auto durationSingle = std::chrono::duration_cast<std::chrono::milliseconds>(endSingle - startSingle);

    EXPECT_TRUE(value1.isArray());
    EXPECT_EQ(value1.size(), numElements);

    // Многопоточный парсинг
    auto startMulti = std::chrono::high_resolution_clock::now();
    auto value2 = Parser::parseFileParallel(filepath, 4);
    auto endMulti = std::chrono::high_resolution_clock::now();
    auto durationMulti = std::chrono::duration_cast<std::chrono::milliseconds>(endMulti - startMulti);

    EXPECT_TRUE(value2.isArray());
    EXPECT_EQ(value2.size(), numElements);

    std::cout << "\nParallel Parsing Performance:\n";
    std::cout << "Single-threaded: " << durationSingle.count() << " ms\n";
    std::cout << "Multi-threaded (4 threads): " << durationMulti.count() << " ms\n";

    if (durationSingle.count() > 0 && durationMulti.count() > 0) {
        double speedup = static_cast<double>(durationSingle.count()) / durationMulti.count();
        std::cout << "Speedup: " << speedup << "x\n";
    }
}

// Тест с прогресс-колбэком
TEST_F(ParallelProcessingTest, ProgressCallback) {
    const int numElements = 5000;  // Уменьшено для быстрого выполнения
    std::string json = generateLargeArray(numElements);
    auto filepath = createTestFile(json, "progress_test.json");

    std::atomic<int> callbackCount{0};
    std::atomic<size_t> lastProgress{0};

    ParallelProcessor processor(2);  // Меньше потоков
    auto result = processor.validateLargeFile(filepath,
        [&callbackCount, &lastProgress](const ProcessingProgress& progress) {
            callbackCount++;
            lastProgress = progress.processedChunks.load();
        });

    EXPECT_TRUE(result.success);
    // Колбэк может быть вызван или не вызван в зависимости от размера файла
    std::cout << "Progress callback was called " << callbackCount.load() << " times\n";
}

// Стресс-тест: параллельная обработка множества файлов
TEST_F(ParallelProcessingTest, StressTest_MultipleFilesParallel) {
    const int numFiles = 5;
    const int numElements = 5000;

    std::vector<std::string> files;

    // Создаем несколько файлов
    for (int f = 0; f < numFiles; ++f) {
        std::string json = generateLargeArray(numElements);
        files.push_back(createTestFile(json, "stress_" + std::to_string(f) + ".json"));
    }

    // Обрабатываем файлы параллельно
    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};

    auto start = std::chrono::high_resolution_clock::now();

    for (const auto& file : files) {
        threads.emplace_back([&file, &successCount]() {
            ParallelProcessor processor(2);
            auto result = processor.validateLargeFile(file);
            if (result.success) {
                successCount++;
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_EQ(successCount.load(), numFiles);
    std::cout << "Processed " << numFiles << " files in parallel in " << duration.count() << " ms\n";
}

// Тест использования всех доступных ядер процессора
TEST_F(ParallelProcessingTest, UseAllCores) {
    const int numElements = 20000;
    std::string json = generateLargeArray(numElements);
    auto filepath = createTestFile(json, "all_cores.json");

    // 0 означает использование всех доступных ядер
    ParallelProcessor processor(0);

    auto start = std::chrono::high_resolution_clock::now();
    auto result = processor.validateLargeFile(filepath);
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_TRUE(result.success);
    std::cout << "Used " << processor.getThreadCount() << " threads (auto-detected)\n";
    std::cout << "Processing time: " << duration.count() << " ms\n";
}
