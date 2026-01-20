#include "Parser.hpp"
#include "JsonValue.hpp"
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <cstdlib>

using namespace json;

// Утилита для получения текущего использования памяти процесса (в KB)
#ifdef __APPLE__
#include <mach/mach.h>

size_t getCurrentMemoryUsage() {
    struct mach_task_basic_info info;
    mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO,
                  (task_info_t)&info, &infoCount) != KERN_SUCCESS) {
        return 0;
    }
    return info.resident_size / 1024;  // Convert to KB
}
#elif __linux__
#include <unistd.h>
#include <fstream>

size_t getCurrentMemoryUsage() {
    std::ifstream statm("/proc/self/statm");
    long size, resident, share, text, lib, data, dt;
    statm >> size >> resident >> share >> text >> lib >> data >> dt;
    return (resident * sysconf(_SC_PAGESIZE)) / 1024;  // Convert to KB
}
#else
size_t getCurrentMemoryUsage() {
    return 0;  // Not implemented for this platform
}
#endif

// Класс для профилирования памяти
class MemoryProfiler {
private:
    size_t startMemory;
    std::string description;

public:
    MemoryProfiler(const std::string& desc) : description(desc) {
        startMemory = getCurrentMemoryUsage();
    }

    ~MemoryProfiler() {
        size_t endMemory = getCurrentMemoryUsage();
        size_t diff = endMemory - startMemory;

        std::cout << std::left << std::setw(50) << description
                  << std::right << std::setw(15) << startMemory << " KB"
                  << std::right << std::setw(15) << endMemory << " KB"
                  << std::right << std::setw(15);

        if (diff > 0) {
            std::cout << "+" << diff << " KB";
        } else {
            std::cout << diff << " KB";
        }
        std::cout << "\n";
    }

    void checkpoint(const std::string& checkpointName) {
        size_t currentMemory = getCurrentMemoryUsage();
        size_t diff = currentMemory - startMemory;

        std::cout << "  └─ " << std::left << std::setw(45) << checkpointName
                  << std::right << std::setw(15) << currentMemory << " KB"
                  << std::right << std::setw(15);

        if (diff > 0) {
            std::cout << "(+" << diff << " KB)";
        } else {
            std::cout << "(" << diff << " KB)";
        }
        std::cout << "\n";
    }
};

// Генерация больших тестовых данных
std::string generateLargeJSON(int numObjects) {
    std::string json = "[";
    for (int i = 0; i < numObjects; ++i) {
        if (i > 0) json += ",";
        json += R"({"id":)" + std::to_string(i) +
                R"(,"name":"User )" + std::to_string(i) +
                R"(","email":"user)" + std::to_string(i) + R"(@example.com",)" +
                R"("age":)" + std::to_string(20 + (i % 50)) +
                R"(,"active":)" + (i % 2 == 0 ? "true" : "false") +
                R"(,"score":)" + std::to_string(i * 3.14) + "}";
    }
    json += "]";
    return json;
}

// Тест копирования vs перемещения
void testCopyVsMove() {
    std::cout << "\n[1] Copy vs Move Performance\n" << std::string(70, '-') << "\n";

    const int numElements = 10000;
    JsonArray arr;
    for (int i = 0; i < numElements; ++i) {
        arr.push_back(JsonValue(i));
    }

    // Тест копирования
    {
        size_t memBefore = getCurrentMemoryUsage();
        auto start = std::chrono::high_resolution_clock::now();

        JsonValue copy = JsonValue(arr);  // Copy constructor

        auto end = std::chrono::high_resolution_clock::now();
        size_t memAfter = getCurrentMemoryUsage();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::cout << "Copy: " << duration.count() << " μs, "
                  << "Memory: +" << (memAfter - memBefore) << " KB\n";
    }

    // Тест перемещения
    {
        JsonArray tempArr;
        for (int i = 0; i < numElements; ++i) {
            tempArr.push_back(JsonValue(i));
        }

        size_t memBefore = getCurrentMemoryUsage();
        auto start = std::chrono::high_resolution_clock::now();

        JsonValue moved = JsonValue(std::move(tempArr));  // Move constructor

        auto end = std::chrono::high_resolution_clock::now();
        size_t memAfter = getCurrentMemoryUsage();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::cout << "Move: " << duration.count() << " μs, "
                  << "Memory: +" << (memAfter - memBefore) << " KB\n";
    }
}

// Анализ использования памяти при парсинге
void analyzeParsingMemory() {
    std::cout << "\n[2] Memory Usage During Parsing\n" << std::string(70, '-') << "\n";
    std::cout << std::left << std::setw(50) << "Operation"
              << std::right << std::setw(15) << "Start"
              << std::right << std::setw(15) << "End"
              << std::right << std::setw(15) << "Difference"
              << "\n" << std::string(95, '-') << "\n";

    std::vector<int> sizes = {100, 1000, 10000};

    for (int size : sizes) {
        std::string json = generateLargeJSON(size);

        {
            MemoryProfiler profiler("Parse " + std::to_string(size) + " objects");
            JsonValue value = Parser::parseString(json);
        }
    }
}

// Профилирование постепенного построения объектов
void profileIncrementalBuilding() {
    std::cout << "\n[3] Incremental Object Building\n" << std::string(70, '-') << "\n";

    const int numElements = 10000;

    {
        MemoryProfiler profiler("Building array incrementally (" + std::to_string(numElements) + " elements)");

        JsonArray arr;
        for (int i = 0; i < numElements; ++i) {
            arr.push_back(JsonValue(i));

            if (i == 1000 || i == 5000 || i == 9999) {
                profiler.checkpoint("After " + std::to_string(i + 1) + " elements");
            }
        }
    }
}

// Анализ утечек памяти при множественных операциях
void detectMemoryLeaks() {
    std::cout << "\n[4] Memory Leak Detection\n" << std::string(70, '-') << "\n";

    size_t initialMemory = getCurrentMemoryUsage();
    std::cout << "Initial memory: " << initialMemory << " KB\n\n";

    // Многократное создание и уничтожение объектов
    const int iterations = 100;
    for (int i = 0; i < iterations; ++i) {
        std::string json = generateLargeJSON(100);
        JsonValue value = Parser::parseString(json);
        // value уничтожается в конце каждой итерации
    }

    size_t finalMemory = getCurrentMemoryUsage();
    std::cout << "Final memory after " << iterations << " iterations: " << finalMemory << " KB\n";
    std::cout << "Memory difference: ";

    if (finalMemory > initialMemory) {
        std::cout << "+" << (finalMemory - initialMemory) << " KB ";

        long long growth = (finalMemory - initialMemory);
        if (growth > 1000) {
            std::cout << "(⚠ Possible memory leak detected!)\n";
        } else {
            std::cout << "(✓ No significant leak)\n";
        }
    } else {
        std::cout << (finalMemory - initialMemory) << " KB (✓ OK)\n";
    }
}

// Сравнение размеров различных структур JSON
void compareStructureSizes() {
    std::cout << "\n[5] Memory Footprint of Different Structures\n" << std::string(70, '-') << "\n";

    const int count = 1000;

    // Array of numbers
    {
        MemoryProfiler profiler("Array of " + std::to_string(count) + " numbers");
        JsonArray arr;
        for (int i = 0; i < count; ++i) {
            arr.push_back(JsonValue(i));
        }
        JsonValue value(std::move(arr));
    }

    // Array of strings
    {
        MemoryProfiler profiler("Array of " + std::to_string(count) + " strings");
        JsonArray arr;
        for (int i = 0; i < count; ++i) {
            arr.push_back(JsonValue("String number " + std::to_string(i)));
        }
        JsonValue value(std::move(arr));
    }

    // Array of objects
    {
        MemoryProfiler profiler("Array of " + std::to_string(count) + " objects");
        JsonArray arr;
        for (int i = 0; i < count; ++i) {
            JsonObject obj;
            obj["id"] = JsonValue(i);
            obj["name"] = JsonValue("Item " + std::to_string(i));
            arr.push_back(JsonValue(std::move(obj)));
        }
        JsonValue value(std::move(arr));
    }
}

void runMemoryBenchmarks() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║          JSON Parser Memory Profiling                             ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════════╝\n";

    testCopyVsMove();
    analyzeParsingMemory();
    profileIncrementalBuilding();
    compareStructureSizes();
    detectMemoryLeaks();

    std::cout << "\n✓ Memory profiling completed!\n\n";

    std::cout << "Tips for optimization:\n";
    std::cout << "  - Use move semantics when possible to avoid copies\n";
    std::cout << "  - Reserve capacity for vectors/arrays when size is known\n";
    std::cout << "  - Reuse JsonValue objects instead of creating new ones\n";
    std::cout << "  - Consider using string_view for read-only string operations\n\n";
}
