// Задание 1: Базовые потоки и синхронизация
// Этот файл реализует параллельное и последовательное вычисление суммы элементов
// большого вектора. Каждый оператор и конструкция подробно прокомментированы.

#include <iostream>              // Включаем заголовочный файл для ввода‑вывода
#include <vector>                // Включаем заголовочный файл для работы с std::vector
#include <thread>                // Включаем заголовочный файл для работы с потоками std::thread
#include <mutex>                 // Включаем заголовочный файл для использования мьютекса std::mutex
#include <chrono>                // Включаем заголовочный файл для измерения времени std::chrono
#include <random>                // Включаем заголовочный файл для генерации случайных чисел

// Класс, который хранит большой вектор и предоставляет методы для вычисления его суммы
class VectorSumCalculator {
private:
    std::vector<int> data;        // Контейнер для хранения случайных целых чисел
    std::mutex sum_mutex;         // Мьютекс для защиты общей суммы в многопоточном режиме

public:
    // Конструктор принимает размер вектора и заполняет его случайными числами
    VectorSumCalculator(size_t size) {
        data.reserve(size);                                      // Резервируем память под размер size, чтобы избежать лишних аллокаций
        std::random_device rd;                                   // Источник случайности для инициализации генератора
        std::mt19937 gen(rd());                                  // Генератор псевдослучайных чисел Mersenne Twister
        std::uniform_int_distribution<int> dist(1, 100);         // Распределение для генерации чисел в диапазоне [1, 100]
        for (size_t i = 0; i < size; ++i) {                      // Цикл по количеству элементов size
            int value = dist(gen);                               // Генерируем очередное случайное число
            data.push_back(value);                               // Добавляем это число в вектор данных
        }
    }

    // Метод для однопоточного вычисления суммы элементов вектора
    long long calculateSingleThreaded() {
        long long sum = 0;                                       // Переменная для накопления суммы (используем 64‑битный тип)
        for (int value : data) {                                 // Перебираем каждый элемент вектора по значению
            sum += value;                                        // Прибавляем текущее значение к сумме
        }
        return sum;                                              // Возвращаем итоговую сумму
    }

    // Метод для многопоточного вычисления суммы
    long long calculateMultiThreaded(int num_threads) {
        long long total_sum = 0;                                 // Общая сумма, которую будем накапливать с защитой
        size_t n = data.size();                                  // Количество элементов в векторе
        size_t chunk_size = n / num_threads;                     // Основной размер блока элементов для каждого потока
        size_t remainder = n % num_threads;                      // Остаток, который распределится между первыми потоками
        std::vector<std::thread> threads;                        // Вектор для хранения объектов потоков
        threads.reserve(num_threads);                            // Резервируем место под num_threads потоков
        // Лямбда‑функция для обработки поддиапазона элементов в отдельном потоке
        auto worker = [this, &total_sum](size_t start, size_t end) {
            long long local_sum = 0;                             // Локальная сумма в рамках одного потока
            for (size_t i = start; i < end; ++i) {               // Перебираем элементы от start до end (не включительно)
                local_sum += data[i];                            // Добавляем значение элемента к локальной сумме
            }
            {
                std::lock_guard<std::mutex> lock(sum_mutex);    // Захватываем мьютекс для обновления общей суммы
                total_sum += local_sum;                          // Прибавляем локальную сумму к общей
            }                                                   // Мьютекс автоматически освобождается при выходе из области действия
        };
        size_t current_start = 0;                               // Начальный индекс для первого потока
        for (int t = 0; t < num_threads; ++t) {                 // Создаём num_threads потоков
            size_t current_chunk = chunk_size + (t < (int)remainder ? 1 : 0); // Добавляем по одному элементу первым потокам для распределения остатка
            size_t current_end = current_start + current_chunk; // Вычисляем конечный индекс (не включительно) для текущего потока
            threads.emplace_back(worker, current_start, current_end); // Создаём поток, передавая ему диапазон
            current_start = current_end;                        // Обновляем начальный индекс для следующего потока
        }
        for (auto &th : threads) {                              // Перебираем все созданные потоки
            th.join();                                          // Ожидаем завершения работы каждого потока
        }
        return total_sum;                                       // Возвращаем итоговую сумму, полученную всеми потоками
    }
};

// Главная функция для демонстрации работы класса VectorSumCalculator
int main() {
    const size_t vector_size = 10000000;                        // Размер вектора: 10 миллионов элементов
    const int num_threads = 4;                                  // Количество потоков для многопоточного расчёта
    VectorSumCalculator calculator(vector_size);                 // Создаём объект калькулятора суммы, заполняющий вектор
    // Замеряем время однопоточного вычисления
    auto start_single = std::chrono::steady_clock::now();       // Сохраняем текущее время перед началом работы
    long long single_sum = calculator.calculateSingleThreaded(); // Вызываем метод однопоточного вычисления суммы
    auto end_single = std::chrono::steady_clock::now();         // Фиксируем время после завершения работы
    auto duration_single = std::chrono::duration_cast<std::chrono::milliseconds>(end_single - start_single); // Вычисляем длительность выполнения в миллисекундах
    // Замеряем время многопоточного вычисления
    auto start_multi = std::chrono::steady_clock::now();        // Начинаем измерять время перед многопоточным расчётом
    long long multi_sum = calculator.calculateMultiThreaded(num_threads); // Вызываем метод многопоточного вычисления суммы
    auto end_multi = std::chrono::steady_clock::now();          // Фиксируем время после завершения
    auto duration_multi = std::chrono::duration_cast<std::chrono::milliseconds>(end_multi - start_multi); // Вычисляем длительность многопоточного расчёта
    // Выводим результаты и время выполнения
    std::cout << "Однопоточная сумма: " << single_sum << std::endl;          // Печатаем результат однопоточного вычисления
    std::cout << "Многопоточная сумма: " << multi_sum << std::endl;          // Печатаем результат многопоточного вычисления
    std::cout << "Время однопоточного вычисления (мс): " << duration_single.count() << std::endl; // Печатаем длительность однопоточной версии
    std::cout << "Время многопоточного вычисления (мс): " << duration_multi.count() << std::endl; // Печатаем длительность многопоточной версии
    return 0;                                                  // Возвращаем 0, что означает успешное завершение программы
}