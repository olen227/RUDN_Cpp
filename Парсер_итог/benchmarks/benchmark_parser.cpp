#include "Parser.hpp"
#include "Lexer.hpp"
#include "Validator.hpp"
#include "ParallelProcessor.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <filesystem>

using namespace json;
namespace fs = std::filesystem;

// Структура для хранения результатов бенчмарка
struct BenchmarkResult {
    std::string name;
    double averageTime;
    double minTime;
    double maxTime;
    double stdDev;
    size_t iterations;
    double throughputMBps;
};

class BenchmarkRunner {
private:
    std::vector<BenchmarkResult> results;

public:
    template<typename Func>
    void runBenchmark(const std::string& name, Func func, size_t iterations = 10, size_t dataSize = 0) {
        std::vector<double> times;
        times.reserve(iterations);

        std::cout << "Running: " << name << " (" << iterations << " iterations)..." << std::flush;

        for (size_t i = 0; i < iterations; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            func();
            auto end = std::chrono::high_resolution_clock::now();

            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            times.push_back(duration.count() / 1000.0);
        }

        double sum = 0.0, minTime = times[0], maxTime = times[0];
        for (double time : times) {
            sum += time;
            minTime = std::min(minTime, time);
            maxTime = std::max(maxTime, time);
        }

        double average = sum / iterations;
        double variance = 0.0;
        for (double time : times) {
            variance += (time - average) * (time - average);
        }
        double stdDev = std::sqrt(variance / iterations);

        double throughput = 0.0;
        if (dataSize > 0 && average > 0) {
            throughput = (dataSize / 1024.0 / 1024.0) / (average / 1000.0);
        }

        results.push_back({name, average, minTime, maxTime, stdDev, iterations, throughput});
        std::cout << " Done! (" << std::fixed << std::setprecision(2) << average << " ms)\n";
    }

    void printResults() const {
        std::cout << "\n" << std::string(100, '=') << "\n";
        std::cout << "BENCHMARK RESULTS\n";
        std::cout << std::string(100, '=') << "\n\n";

        std::cout << std::left << std::setw(45) << "Benchmark"
                  << std::right << std::setw(12) << "Avg (ms)"
                  << std::right << std::setw(12) << "Min (ms)"
                  << std::right << std::setw(12) << "Max (ms)"
                  << std::right << std::setw(12) << "StdDev"
                  << std::right << std::setw(12) << "MB/s"
                  << "\n";
        std::cout << std::string(100, '-') << "\n";

        for (const auto& result : results) {
            std::cout << std::left << std::setw(45) << result.name
                      << std::right << std::setw(12) << std::fixed << std::setprecision(3) << result.averageTime
                      << std::right << std::setw(12) << result.minTime
                      << std::right << std::setw(12) << result.maxTime
                      << std::right << std::setw(12) << result.stdDev;
            if (result.throughputMBps > 0) {
                std::cout << std::right << std::setw(12) << std::fixed << std::setprecision(2) << result.throughputMBps;
            } else {
                std::cout << std::right << std::setw(12) << "-";
            }
            std::cout << "\n";
        }

        std::cout << std::string(100, '=') << "\n\n";
    }
};

// Генерация тестовых данных
std::string generateArray(int size) {
    std::string json = "[";
    for (int i = 0; i < size; ++i) {
        if (i > 0) json += ",";
        json += std::to_string(i);
    }
    json += "]";
    return json;
}

std::string generateObject(int size) {
    std::string json = "{";
    for (int i = 0; i < size; ++i) {
        if (i > 0) json += ",";
        json += "\"key" + std::to_string(i) + "\":" + std::to_string(i);
    }
    json += "}";
    return json;
}

std::string generateComplexObject(int size) {
    std::string json = "{\"users\":[";
    for (int i = 0; i < size; ++i) {
        if (i > 0) json += ",";
        json += R"({"id":)" + std::to_string(i) +
                R"(,"name":"User)" + std::to_string(i) +
                R"(","email":"user)" + std::to_string(i) + R"(@example.com"})";
    }
    json += "]}";
    return json;
}

void runParserBenchmarks() {
    BenchmarkRunner runner;

    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║          JSON Parser Performance Benchmarks                        ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════════╝\n\n";

    // === Бенчмарки Lexer ===
    std::cout << "\n[1] Lexer Benchmarks\n" << std::string(50, '-') << "\n";

    std::string smallArray = generateArray(100);
    runner.runBenchmark("Lexer: Small Array (100 elements)", [&smallArray]() {
        Lexer lexer(smallArray);
        lexer.tokenize();
    }, 100);

    std::string mediumArray = generateArray(1000);
    runner.runBenchmark("Lexer: Medium Array (1000 elements)", [&mediumArray]() {
        Lexer lexer(mediumArray);
        lexer.tokenize();
    }, 100);

    std::string largeArray = generateArray(10000);
    runner.runBenchmark("Lexer: Large Array (10000 elements)", [&largeArray]() {
        Lexer lexer(largeArray);
        lexer.tokenize();
    }, 50);

    // === Бенчмарки Parser ===
    std::cout << "\n[2] Parser Benchmarks\n" << std::string(50, '-') << "\n";

    runner.runBenchmark("Parser: Small Array (100)", [&smallArray]() {
        Parser::parseString(smallArray);
    }, 100);

    runner.runBenchmark("Parser: Medium Array (1000)", [&mediumArray]() {
        Parser::parseString(mediumArray);
    }, 100);

    runner.runBenchmark("Parser: Large Array (10000)", [&largeArray]() {
        Parser::parseString(largeArray);
    }, 50);

    std::string complexObj = generateComplexObject(1000);
    runner.runBenchmark("Parser: Complex Objects (1000)", [&complexObj]() {
        Parser::parseString(complexObj);
    }, 50);

    // === Бенчмарки Validator ===
    std::cout << "\n[3] Validator Benchmarks\n" << std::string(50, '-') << "\n";

    Validator validator;
    runner.runBenchmark("Validator: Small Array (100)", [&smallArray, &validator]() {
        validator.validate(smallArray);
    }, 100);

    runner.runBenchmark("Validator: Medium Array (1000)", [&mediumArray, &validator]() {
        validator.validate(mediumArray);
    }, 100);

    runner.runBenchmark("Validator: Large Array (10000)", [&largeArray, &validator]() {
        validator.validate(largeArray);
    }, 50);

    // === Сравнение однопоточной и многопоточной версий ===
    std::cout << "\n[4] Single-threaded vs Multi-threaded\n" << std::string(50, '-') << "\n";

    // Создаем временный файл для тестирования
    std::string testDir = "benchmark_temp";
    fs::create_directory(testDir);

    std::string bigArray = generateComplexObject(50000);
    std::string filepath = testDir + "/big_test.json";
    std::ofstream file(filepath);
    file << bigArray;
    file.close();

    size_t fileSize = bigArray.size();

    runner.runBenchmark("Single-threaded Parse (50k objects)", [&filepath]() {
        Parser::parseFile(filepath);
    }, 5, fileSize);

    runner.runBenchmark("Multi-threaded Parse (50k, 4 threads)", [&filepath]() {
        Parser::parseFileParallel(filepath, 4);
    }, 5, fileSize);

    runner.runBenchmark("Multi-threaded Parse (50k, auto)", [&filepath]() {
        Parser::parseFileParallel(filepath, 0);
    }, 5, fileSize);

    // === Параллельная валидация ===
    std::cout << "\n[5] Parallel Validation\n" << std::string(50, '-') << "\n";

    runner.runBenchmark("Single-threaded Validation", [&filepath, &validator]() {
        validator.validateFile(filepath);
    }, 5, fileSize);

    ParallelProcessor processor2(2);
    runner.runBenchmark("Parallel Validation (2 threads)", [&filepath, &processor2]() {
        processor2.validateLargeFile(filepath);
    }, 5, fileSize);

    ParallelProcessor processor4(4);
    runner.runBenchmark("Parallel Validation (4 threads)", [&filepath, &processor4]() {
        processor4.validateLargeFile(filepath);
    }, 5, fileSize);

    ParallelProcessor processor8(8);
    runner.runBenchmark("Parallel Validation (8 threads)", [&filepath, &processor8]() {
        processor8.validateLargeFile(filepath);
    }, 5, fileSize);

    // === Разные размеры данных ===
    std::cout << "\n[6] Scaling with Data Size\n" << std::string(50, '-') << "\n";

    for (int size : {100, 500, 1000, 5000, 10000}) {
        std::string data = generateArray(size);
        runner.runBenchmark("Parse Array (" + std::to_string(size) + " elements)",
            [&data]() { Parser::parseString(data); }, 20, data.size());
    }

    // Очистка
    fs::remove_all(testDir);

    // Вывод результатов
    runner.printResults();

    std::cout << "\n✓ Parser benchmarks completed successfully!\n\n";
}
