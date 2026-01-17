// Задание 4: Асинхронные вычисления с std::async
// Этот файл содержит реализацию вычисления числа π методом Монте‑Карло
// с использованием std::async и std::future. Все строки снабжены комментариями.

#include <iostream>                 // Заголовочный файл для ввода‑вывода
#include <future>                   // Заголовочный файл для std::async и std::future
#include <random>                   // Заголовочный файл для генерации случайных чисел
#include <vector>                   // Заголовочный файл для std::vector
#include <chrono>                   // Заголовочный файл для измерения времени
#include <exception>                // Заголовочный файл для обработки исключений

// Класс PiCalculator обеспечивает методы для вычисления π
class PiCalculator {
public:
    // Метод, вычисляющий часть значения π с использованием total_points случайных точек
    double calculatePiPortion(int total_points) {
        std::random_device rd;                           // Источник случайности для генератора
        std::mt19937 gen(rd());                          // Генератор псевдослучайных чисел Mersenne Twister
        std::uniform_real_distribution<double> dist(0.0, 1.0); // Распределение для получения чисел от 0 до 1
        int inside = 0;                                  // Счётчик точек, попавших внутрь четверти круга радиуса 1
        for (int i = 0; i < total_points; ++i) {         // Генерируем total_points случайных точек
            double x = dist(gen);                        // Случайная координата x в диапазоне [0, 1]
            double y = dist(gen);                        // Случайная координата y в диапазоне [0, 1]
            if (x * x + y * y <= 1.0) {                  // Проверяем, находится ли точка внутри четверти круга
                ++inside;                                // Если внутри, увеличиваем счётчик
            }
        }
        // Расчёт приближения π для этой порции: 4 * (количество попаданий / общее количество точек)
        return 4.0 * static_cast<double>(inside) / static_cast<double>(total_points);
    }
    // Метод, выполняющий параллельное вычисление π, используя num_tasks асинхронных задач
    double calculatePiParallel(int total_points, int num_tasks) {
        if (num_tasks <= 0) {                            // Проверяем корректность числа задач
            throw std::invalid_argument("Количество задач должно быть положительным");
        }
        std::vector<std::future<double>> futures;        // Вектор для хранения futures, возвращающих результаты задач
        futures.reserve(num_tasks);                      // Резервируем место в векторе
        int points_per_task = total_points / num_tasks;  // Количество точек для каждой задачи (целая часть)
        int remainder = total_points % num_tasks;        // Оставшиеся точки, которые нужно распределить
        // Создаём асинхронные задачи
        for (int i = 0; i < num_tasks; ++i) {            // Запускаем num_tasks задач
            int current_points = points_per_task + (i < remainder ? 1 : 0); // Распределяем остаток точек между первыми задачами
            // Создаём задачу, указывая политику std::launch::async для гарантированного создания нового потока
            futures.emplace_back(std::async(std::launch::async, &PiCalculator::calculatePiPortion, this, current_points));
        }
        // Собираем результаты и обрабатываем возможные исключения
        double sum_pi = 0.0;                             // Сумма частичных значений π
        for (auto& fut : futures) {                      // Перебираем все futures
            try {
                double portion = fut.get();              // Получаем результат задачи; если задача завершилась исключением, оно будет выброшено здесь
                sum_pi += portion;                       // Добавляем полученное приближение π к сумме
            } catch (const std::exception& e) {          // Ловим возможные исключения из задач
                std::cerr << "Ошибка в асинхронной задаче: " << e.what() << std::endl; // Сообщаем об ошибке
            }
        }
        // Среднее значение приближений π (если задач было больше нуля)
        return num_tasks > 0 ? sum_pi / static_cast<double>(num_tasks) : 0.0;
    }
};

// Главная функция демонстрирует работу PiCalculator
int main() {
    const int total_points = 1000000;                    // Общее количество точек для метода Монте‑Карло
    const int num_tasks = 8;                             // Количество асинхронных задач
    PiCalculator calculator;                             // Создаём объект PiCalculator
    // Сначала выполняем последовательный расчёт для сравнения
    auto start_single = std::chrono::steady_clock::now(); // Фиксируем время до последовательного расчёта
    double pi_single = calculator.calculatePiPortion(total_points); // Последовательное вычисление π одной задачей
    auto end_single = std::chrono::steady_clock::now();   // Время после завершения
    auto duration_single = std::chrono::duration_cast<std::chrono::milliseconds>(end_single - start_single); // Длительность последовательного расчёта
    // Выполняем параллельный расчёт с использованием std::async
    auto start_parallel = std::chrono::steady_clock::now(); // Время до запуска параллельных задач
    double pi_parallel = calculator.calculatePiParallel(total_points, num_tasks); // Параллельное вычисление π
    auto end_parallel = std::chrono::steady_clock::now();   // Время после завершения всех задач
    auto duration_parallel = std::chrono::duration_cast<std::chrono::milliseconds>(end_parallel - start_parallel); // Длительность параллельного расчёта
    // Выводим результаты пользователю
    std::cout << "Последовательное вычисление π: " << pi_single << std::endl;        // Печатаем результат последовательного вычисления
    std::cout << "Параллельное вычисление π: " << pi_parallel << std::endl;         // Печатаем результат параллельного вычисления
    std::cout << "Время последовательного вычисления (мс): " << duration_single.count() << std::endl; // Время последовательного расчёта
    std::cout << "Время параллельного вычисления (мс): " << duration_parallel.count() << std::endl; // Время параллельного расчёта
    return 0;                                            // Возвращаем 0, что означает успешное завершение программы
}