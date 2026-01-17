#ifndef PARALLEL_PROCESSOR_HPP
#define PARALLEL_PROCESSOR_HPP

#include "Validator.hpp"
#include "SystemInfo.hpp"
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <future>
#include <queue>
#include <condition_variable>

namespace json {

// Результат параллельной обработки
struct ParallelResult {
    bool success;
    size_t totalChunks;
    size_t processedChunks;
    size_t totalErrors;
    double totalTimeMs;
    double throughputMBps;
    std::vector<ValidationError> errors;
};

// Прогресс обработки
struct ProcessingProgress {
    std::atomic<size_t> processedChunks{0};
    std::atomic<size_t> totalChunks{0};
    std::atomic<size_t> processedBytes{0};
    std::atomic<size_t> totalBytes{0};
    std::atomic<size_t> errorsFound{0};
    std::atomic<bool> isComplete{false};
};

// Класс для параллельной обработки JSON
class ParallelProcessor {
private:
    unsigned int m_threadCount;
    std::mutex m_mutex;
    std::mutex m_errorMutex;
    ProcessingProgress m_progress;

    // Разбить файл на чанки по границам JSON-значений (для массива)
    std::vector<std::pair<size_t, size_t>> splitIntoChunks(
        const std::string& content, size_t chunkCount);

    // Найти конец JSON-объекта/элемента массива
    size_t findJsonBoundary(const std::string& content, size_t startPos);

public:
    explicit ParallelProcessor(unsigned int threadCount = 0);

    // Установить количество потоков
    void setThreadCount(unsigned int count);
    unsigned int getThreadCount() const { return m_threadCount; }

    // Параллельная валидация большого файла
    ParallelResult validateLargeFile(const std::string& filename,
                                     std::function<void(const ProcessingProgress&)> progressCallback = nullptr);

    // Параллельная валидация содержимого (для массивов JSON)
    ParallelResult validateContent(const std::string& content,
                                   std::function<void(const ProcessingProgress&)> progressCallback = nullptr);

    // Получить прогресс
    const ProcessingProgress& getProgress() const { return m_progress; }

    // Сброс прогресса
    void resetProgress();
};

// Класс для параллельной генерации больших JSON файлов
class ParallelGenerator {
private:
    unsigned int m_threadCount;
    std::mutex m_fileMutex;
    std::atomic<size_t> m_generatedBytes{0};
    std::atomic<size_t> m_targetBytes{0};

    // Генерация одного чанка
    std::string generateChunk(size_t targetSize, int depth, int seed, int errorProbability);

public:
    explicit ParallelGenerator(unsigned int threadCount = 0);

    // Установить количество потоков
    void setThreadCount(unsigned int count);

    // Генерация большого файла заданного размера
    bool generateLargeFile(const std::string& filename,
                          size_t targetSizeBytes,
                          int depth,
                          int errorProbability,
                          std::function<void(size_t current, size_t total)> progressCallback = nullptr);

    // Получить прогресс генерации
    size_t getGeneratedBytes() const { return m_generatedBytes; }
    size_t getTargetBytes() const { return m_targetBytes; }
};

} // namespace json

#endif // PARALLEL_PROCESSOR_HPP
