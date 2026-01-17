// Задание 5: Пул потоков и выполнение задач
// В этом файле реализован простой пул потоков, который позволяет добавлять
// задания в очередь для асинхронного выполнения. Также демонстрируется работа
// пула на примере вычисления факториалов различных чисел. Каждая строка кода
// сопровождается подробными комментариями.

#include <iostream>                 // Заголовочный файл для ввода‑вывода
#include <thread>                   // Заголовочный файл для std::thread
#include <vector>                   // Заголовочный файл для std::vector
#include <queue>                    // Заголовочный файл для std::queue
#include <future>                   // Заголовочный файл для std::packaged_task и std::future
#include <functional>               // Заголовочный файл для std::function
#include <mutex>                    // Заголовочный файл для std::mutex
#include <condition_variable>       // Заголовочный файл для std::condition_variable

// Класс ThreadPool представляет пул потоков, выполняющий задачи из очереди
class ThreadPool {
private:
    std::vector<std::thread> workers;                 // Вектор рабочих потоков
    std::queue<std::function<void()>> tasks;          // Очередь задач, каждая задача — это вызываемый объект без аргументов
    std::mutex queue_mutex;                           // Мьютекс для защиты очереди задач
    std::condition_variable condition;                // Условная переменная для ожидания появления задач
    bool stop = false;                               // Флаг для остановки пула

public:
    // Конструктор принимает количество потоков и создаёт рабочие потоки
    ThreadPool(size_t num_threads) {
        for (size_t i = 0; i < num_threads; ++i) {   // Создаём num_threads потоков
            workers.emplace_back([this]() {          // Добавляем новый поток в вектор
                while (true) {                       // Бесконечный цикл обработки задач
                    std::function<void()> task;      // Локальная переменная для очередной задачи
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex); // Захватываем мьютекс для доступа к очереди
                        // Ожидаем, пока появится задача или поступит сигнал остановки
                        this->condition.wait(lock, [this](){ return this->stop || !this->tasks.empty(); });
                        if (this->stop && this->tasks.empty()) { // Если пул остановлен и задач нет
                            return;              // Выходим из потока
                        }
                        task = std::move(this->tasks.front()); // Извлекаем задачу из начала очереди
                        this->tasks.pop();      // Удаляем задачу из очереди
                    } // Освобождаем мьютекс перед выполнением задачи
                    task();                     // Выполняем задачу вне защищённого блока
                }
            });
        }
    }
    // Метод submit принимает вызываемый объект и его аргументы, возвращает future с результатом
    template<class F, class... Args>
    auto submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        using return_type = decltype(f(args...));     // Определяем тип возвращаемого значения функции
        auto task = std::make_shared<std::packaged_task<return_type()>>( // Оборачиваем функцию и её аргументы в packaged_task
            std::bind(std::forward<F>(f), std::forward<Args>(args)...) // Используем std::bind для захвата аргументов
        );
        std::future<return_type> res = task->get_future(); // Получаем future для packaged_task
        {
            std::lock_guard<std::mutex> lock(queue_mutex); // Захватываем мьютекс для добавления задачи в очередь
            if (stop) {                                    // Если пул остановлен, бросаем исключение
                throw std::runtime_error("submit on stopped ThreadPool");
            }
            tasks.emplace([task](){ (*task)(); });         // Помещаем задачу в очередь, оборачивая её в лямбду без параметров
        }                                                  // Освобождаем мьютекс
        condition.notify_one();                            // Будим один поток, ожидающий задач
        return res;                                        // Возвращаем future, позволяющий получить результат
    }
    // Деструктор корректно завершает работу пула
    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lock(queue_mutex); // Захватываем мьютекс перед изменением флага
            stop = true;                                   // Устанавливаем флаг остановки
        }
        condition.notify_all();                            // Оповещаем все потоки, чтобы они завершились
        for (std::thread &worker : workers) {             // Перебираем все рабочие потоки
            if (worker.joinable()) {                     // Проверяем, можно ли присоединить поток
                worker.join();                           // Присоединяем поток (ожидаем завершения)
            }
        }
    }
};

// Функция для демонстрации: вычисление факториала числа n
unsigned long long factorial(int n) {
    if (n < 0) {                                       // Проверяем корректность входного значения
        throw std::invalid_argument("n must be non-negative");
    }
    unsigned long long result = 1;                    // Начинаем результат с 1
    for (int i = 2; i <= n; ++i) {                    // Умножаем result на все числа от 2 до n
        result *= static_cast<unsigned long long>(i); // Умножаем результат на i (приводя к типу)
    }
    return result;                                    // Возвращаем вычисленный факториал
}

// Главная функция демонстрирует использование пула потоков
int main() {
    const size_t num_threads = 4;                    // Количество потоков в пуле
    ThreadPool pool(num_threads);                    // Создаём объект пула с указанным количеством потоков
    // Вектор чисел, для которых будем вычислять факториалы
    std::vector<int> numbers = {5, 7, 10, 12, 15, 20}; // Несколько примеров входных значений
    // Вектор для хранения futures, соответствующих вызовам submit
    std::vector<std::future<unsigned long long>> futures;
    futures.reserve(numbers.size());                  // Резервируем место в векторе для всех future
    // Отправляем задачи в пул
    for (int n : numbers) {                          // Перебираем каждое число в массиве
        // Отправляем вычисление факториала в пул, сохраняем полученный future
        futures.push_back(pool.submit(factorial, n));
    }
    // Получаем и выводим результаты
    for (size_t i = 0; i < numbers.size(); ++i) {     // Перебираем индексы чисел
        try {
            unsigned long long result = futures[i].get(); // Получаем результат вычисления факториала через future
            std::cout << "Факториал(" << numbers[i] << ") = " << result << std::endl; // Выводим результат на экран
        } catch (const std::exception& e) {            // Ловим возможные исключения
            std::cerr << "Ошибка при вычислении факториала: " << e.what() << std::endl; // Сообщаем об ошибке
        }
    }
    return 0;                                        // Возвращаем 0, что означает успешное завершение программы
}