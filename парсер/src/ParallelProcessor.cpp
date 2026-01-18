#include "ParallelProcessor.hpp"
#include "Generator.hpp"
#include "Lexer.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <cstring>

namespace json {

// ==================== ParallelProcessor ====================

ParallelProcessor::ParallelProcessor(unsigned int threadCount) {
    if (threadCount == 0) {
        m_threadCount = std::thread::hardware_concurrency();
        if (m_threadCount == 0) m_threadCount = 1;
    } else {
        m_threadCount = threadCount;
    }
}

void ParallelProcessor::setThreadCount(unsigned int count) {
    m_threadCount = count > 0 ? count : 1;
}

void ParallelProcessor::resetProgress() {
    m_progress.processedChunks = 0;
    m_progress.totalChunks = 0;
    m_progress.processedBytes = 0;
    m_progress.totalBytes = 0;
    m_progress.errorsFound = 0;
    m_progress.isComplete = false;
}

size_t ParallelProcessor::findJsonBoundary(const std::string& content, size_t startPos) {
    int braceDepth = 0;
    int bracketDepth = 0;
    bool inString = false;
    bool escaped = false;

    for (size_t i = startPos; i < content.size(); ++i) {
        char c = content[i];

        if (escaped) {
            escaped = false;
            continue;
        }

        if (c == '\\' && inString) {
            escaped = true;
            continue;
        }

        if (c == '"') {
            inString = !inString;
            continue;
        }

        if (inString) continue;

        if (c == '{') braceDepth++;
        else if (c == '}') braceDepth--;
        else if (c == '[') bracketDepth++;
        else if (c == ']') bracketDepth--;

        // Нашли конец элемента (на уровне 1)
        if (braceDepth == 0 && bracketDepth == 1 && c == ',') {
            return i + 1;
        }

        // Конец массива
        if (braceDepth == 0 && bracketDepth == 0 && c == ']') {
            return i;
        }
    }

    return content.size();
}

std::vector<std::pair<size_t, size_t>> ParallelProcessor::splitIntoChunks(
    const std::string& content, size_t chunkCount) {

    std::vector<std::pair<size_t, size_t>> chunks;

    // Находим начало массива
    size_t start = content.find('[');
    if (start == std::string::npos) {
        // Не массив - обрабатываем целиком
        chunks.emplace_back(0, content.size());
        return chunks;
    }

    start++; // Пропускаем '['

    // Пропускаем пробелы
    while (start < content.size() && std::isspace(content[start])) {
        start++;
    }

    size_t chunkSize = (content.size() - start) / chunkCount;
    if (chunkSize < 1000) chunkSize = 1000; // Минимум 1KB на чанк

    size_t currentPos = start;

    while (currentPos < content.size()) {
        size_t targetEnd = currentPos + chunkSize;
        if (targetEnd >= content.size()) {
            targetEnd = content.size();
        }

        // Находим границу JSON-элемента
        size_t actualEnd = findJsonBoundary(content, targetEnd);
        if (actualEnd <= currentPos) {
            actualEnd = content.size();
        }

        chunks.emplace_back(currentPos, actualEnd);
        currentPos = actualEnd;

        // Пропускаем пробелы и запятые
        while (currentPos < content.size() &&
               (std::isspace(content[currentPos]) || content[currentPos] == ',')) {
            currentPos++;
        }

        if (currentPos >= content.size() - 1) break;
    }

    return chunks;
}

ParallelResult ParallelProcessor::validateLargeFile(
    const std::string& filename,
    std::function<void(const ProcessingProgress&)> progressCallback) {

    ParallelResult result;
    result.success = false;
    result.totalChunks = 0;
    result.processedChunks = 0;
    result.totalErrors = 0;
    result.totalTimeMs = 0;

    resetProgress();

    // Читаем файл
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        ValidationError err(0, 0, "Не удалось открыть файл: " + filename, "");
        result.errors.push_back(err);
        return result;
    }

    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string content(fileSize, '\0');
    file.read(&content[0], fileSize);
    file.close();

    m_progress.totalBytes = fileSize;

    return validateContent(content, progressCallback);
}

ParallelResult ParallelProcessor::validateContent(
    const std::string& content,
    std::function<void(const ProcessingProgress&)> progressCallback) {

    ParallelResult result;
    result.success = true;
    result.totalErrors = 0;

    resetProgress();
    m_progress.totalBytes = content.size();

    auto startTime = std::chrono::high_resolution_clock::now();

    // Разбиваем на чанки (границы элементов массива)
    auto chunks = splitIntoChunks(content, m_threadCount);
    result.totalChunks = chunks.size();
    m_progress.totalChunks = chunks.size();

    // Если всего один чанк, используем однопоточную валидацию
    if (chunks.size() == 1) {
        Validator validator(false);
        auto validationResult = validator.validate(content);

        result.errors = std::move(validationResult.errors);
        result.processedChunks = 1;
        result.totalErrors = result.errors.size();
        result.success = result.errors.empty();

        auto endTime = std::chrono::high_resolution_clock::now();
        result.totalTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

        if (result.totalTimeMs > 0) {
            result.throughputMBps = (content.size() / (1024.0 * 1024.0)) / (result.totalTimeMs / 1000.0);
        }

        m_progress.processedChunks = 1;
        m_progress.processedBytes = content.size();
        m_progress.errorsFound = result.totalErrors;
        m_progress.isComplete = true;

        return result;
    }

    std::vector<std::thread> threads;
    std::vector<std::vector<ValidationError>> threadErrors(chunks.size());
    std::mutex errorMutex;

    // Запускаем потоки для валидации элементов
    for (size_t i = 0; i < chunks.size(); ++i) {
        threads.emplace_back([this, &content, &chunks, &threadErrors, &errorMutex, i, progressCallback]() {
            auto& chunk = chunks[i];
            std::string chunkContent = content.substr(chunk.first, chunk.second - chunk.first);

            // Оборачиваем чанк в массив, чтобы сделать его валидным JSON
            std::string wrappedContent = "[" + chunkContent + "]";

            // Валидируем чанк как массив JSON элементов
            Validator validator(false);
            auto chunkResult = validator.validate(wrappedContent);

            // Корректируем позиции ошибок относительно начала файла
            size_t lineOffset = 0;
            for (size_t j = 0; j < chunk.first && j < content.size(); ++j) {
                if (content[j] == '\n') lineOffset++;
            }

            for (auto& err : chunkResult.errors) {
                err.line += lineOffset;
            }

            {
                std::lock_guard<std::mutex> lock(errorMutex);
                threadErrors[i] = std::move(chunkResult.errors);
            }

            // Атомарно обновляем прогресс
            m_progress.processedChunks.fetch_add(1, std::memory_order_relaxed);
            m_progress.processedBytes.fetch_add(chunkContent.size(), std::memory_order_relaxed);
            m_progress.errorsFound.fetch_add(threadErrors[i].size(), std::memory_order_relaxed);

            if (progressCallback) {
                progressCallback(m_progress);
            }
        });
    }

    // Ждём завершения всех потоков
    for (auto& t : threads) {
        t.join();
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    result.totalTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    // Собираем все ошибки
    for (auto& errors : threadErrors) {
        for (auto& err : errors) {
            result.errors.push_back(std::move(err));
        }
    }

    result.processedChunks = m_progress.processedChunks.load();
    result.totalErrors = result.errors.size();
    result.success = result.errors.empty();

    // Вычисляем пропускную способность
    if (result.totalTimeMs > 0) {
        result.throughputMBps = (content.size() / (1024.0 * 1024.0)) / (result.totalTimeMs / 1000.0);
    }

    m_progress.isComplete = true;

    return result;
}

// ==================== ParallelGenerator ====================

ParallelGenerator::ParallelGenerator(unsigned int threadCount) {
    if (threadCount == 0) {
        m_threadCount = std::thread::hardware_concurrency();
        if (m_threadCount == 0) m_threadCount = 1;
    } else {
        m_threadCount = threadCount;
    }
}

void ParallelGenerator::setThreadCount(unsigned int count) {
    m_threadCount = count > 0 ? count : 1;
}

std::string ParallelGenerator::generateChunk(size_t targetSize, int depth, int seed, int errorProbability) {
    Generator generator(seed);
    GeneratorOptions opts;
    opts.maxDepth = depth;
    opts.maxArraySize = 10;
    opts.maxObjectKeys = 8;
    opts.errorProbability = errorProbability;
    opts.compactOutput = true;
    generator.setOptions(opts);

    std::ostringstream oss;
    size_t currentSize = 0;
    bool firstElement = true;

    while (currentSize < targetSize) {
        std::string element = generator.generate();

        // Добавляем запятую ПЕРЕД элементом (кроме первого)
        if (!firstElement) {
            oss << ",\n";
            currentSize += 2;
        }
        firstElement = false;

        oss << element;
        currentSize += element.size();
    }

    return oss.str();
}

bool ParallelGenerator::generateLargeFile(
    const std::string& filename,
    size_t targetSizeBytes,
    int depth,
    int errorProbability,
    std::function<void(size_t current, size_t total)> progressCallback) {

    m_generatedBytes = 0;
    m_targetBytes = targetSizeBytes;

    // Открываем файл для записи
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    // Начало массива
    file << "[\n";
    m_generatedBytes += 2;

    const size_t chunkSize = 4 * 1024 * 1024; // 4MB
    std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));

    // Записываем результаты по мере готовности
    bool first = true;
    while (m_generatedBytes < targetSizeBytes) {
        size_t remaining = targetSizeBytes > m_generatedBytes
            ? targetSizeBytes - m_generatedBytes
            : 0;
        if (remaining == 0) {
            break;
        }

        size_t batchCount = std::min(static_cast<size_t>(m_threadCount),
            (remaining + chunkSize - 1) / chunkSize);

        std::vector<std::future<std::string>> futures;
        futures.reserve(batchCount);

        for (size_t i = 0; i < batchCount; ++i) {
            size_t thisChunkSize = std::min(chunkSize, remaining);
            remaining -= thisChunkSize;

            futures.push_back(std::async(std::launch::async,
                &ParallelGenerator::generateChunk, this, thisChunkSize, depth, rng(), errorProbability));
        }

        for (auto& future : futures) {
            std::string chunk = future.get();

            std::lock_guard<std::mutex> lock(m_fileMutex);

            if (!first) {
                file << ",\n";
                m_generatedBytes += 2;
            }
            first = false;

            file << chunk;
            m_generatedBytes += chunk.size();

            if (progressCallback) {
                progressCallback(m_generatedBytes, targetSizeBytes);
            }
        }
    }

    // Конец массива
    file << "\n]";
    m_generatedBytes += 2;

    file.close();

    return true;
}

} // namespace json
