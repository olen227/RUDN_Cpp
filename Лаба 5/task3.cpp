// Задание 3: Параллельная обработка «изображений» (матриц)
// В этом файле реализован класс ImageProcessor, который выполняет фильтр размытия
// как в однопоточном, так и в многопоточном режимах. Пример в main демонстрирует
// использование методов и измеряет время выполнения. Все строки прокомментированы.

#include <iostream>                 // Заголовочный файл для ввода‑вывода
#include <vector>                   // Заголовочный файл для std::vector
#include <thread>                   // Заголовочный файл для std::thread
#include <random>                   // Заголовочный файл для генерации случайных чисел
#include <chrono>                   // Заголовочный файл для измерения времени

// Класс, представляющий изображение и методы для его обработки
class ImageProcessor {
private:
    std::vector<std::vector<int>> image; // Двумерный вектор, представляющий пиксели изображения
    int width;                           // Ширина изображения
    int height;                          // Высота изображения

public:
    // Конструктор принимает размеры и заполняет матрицу случайными значениями
    ImageProcessor(int w, int h) : width(w), height(h) {
        image.assign(height, std::vector<int>(width, 0));      // Создаём матрицу размера height x width, заполненную нулями
        std::random_device rd;                                 // Источник случайности для генератора
        std::mt19937 gen(rd());                                // Генератор псевдослучайных чисел
        std::uniform_int_distribution<int> dist(0, 255);       // Распределение яркостей от 0 до 255
        for (int y = 0; y < height; ++y) {                     // Проходим по строкам
            for (int x = 0; x < width; ++x) {                  // Проходим по столбцам
                image[y][x] = dist(gen);                       // Заполняем каждый пиксель случайным значением яркости
            }
        }
    }
    // Функция размытия для одного пикселя: усредняет соседние значения в окне 3x3
    int blurPixel(int x, int y, const std::vector<std::vector<int>>& src) const {
        int sum = 0;                                          // Переменная для суммы яркостей соседей
        int count = 0;                                        // Количество учтённых пикселей
        for (int dy = -1; dy <= 1; ++dy) {                    // Проходим по вертикальному смещению в окне 3x3
            for (int dx = -1; dx <= 1; ++dx) {                // Проходим по горизонтальному смещению
                int nx = x + dx;                              // Координата соседнего пикселя по оси X
                int ny = y + dy;                              // Координата соседнего пикселя по оси Y
                if (nx >= 0 && nx < width && ny >= 0 && ny < height) { // Проверяем, что сосед находится в пределах изображения
                    sum += src[ny][nx];                       // Добавляем яркость соседнего пикселя к сумме
                    ++count;                                  // Увеличиваем счётчик пикселей
                }
            }
        }
        return count > 0 ? sum / count : src[y][x];           // Возвращаем среднее значение, если есть соседи, иначе исходную яркость
    }
    // Метод для однопоточного применения фильтра размытия ко всему изображению
    void applyFilterSingleThread() {
        std::vector<std::vector<int>> src = image;            // Копируем исходное изображение
        std::vector<std::vector<int>> dst = image;            // Создаём матрицу для результата (той же размерности)
        for (int y = 0; y < height; ++y) {                    // Перебираем строки
            for (int x = 0; x < width; ++x) {                 // Перебираем столбцы
                dst[y][x] = blurPixel(x, y, src);             // Вычисляем размытое значение для пикселя (x, y)
            }
        }
        image = std::move(dst);                               // Заменяем исходное изображение размытым
    }
    // Метод для многопоточного применения фильтра размытия
    void applyFilterMultiThread(int num_threads) {
        std::vector<std::vector<int>> src = image;            // Копия исходного изображения
        std::vector<std::vector<int>> dst = image;            // Матрица для результата
        std::vector<std::thread> threads;                     // Вектор для хранения рабочих потоков
        threads.reserve(num_threads);                         // Резервируем место для потоков
        int rows_per_thread = height / num_threads;           // Основное количество строк на каждый поток
        int remainder = height % num_threads;                 // Остаток строк, который распределяем между первыми потоками
        // Лямбда‑функция для обработки определённого диапазона строк
        auto worker = [this, &src, &dst](int start_row, int end_row) {
            for (int y = start_row; y < end_row; ++y) {       // Перебираем строки, закреплённые за этим потоком
                for (int x = 0; x < width; ++x) {             // Перебираем каждый столбец
                    dst[y][x] = blurPixel(x, y, src);         // Сохраняем размытый результат в соответствующей позиции
                }
            }
        };
        int current_start = 0;                                // Начальная строка для первого потока
        for (int t = 0; t < num_threads; ++t) {               // Создаём num_threads потоков
            int block = rows_per_thread + (t < remainder ? 1 : 0); // Количество строк, которое этот поток будет обрабатывать
            int current_end = current_start + block;          // Конечная строка (не включительно)
            threads.emplace_back(worker, current_start, current_end); // Создаём поток, передавая диапазон строк
            current_start = current_end;                      // Обновляем начальную строку для следующего потока
        }
        for (auto& th : threads) {                           // Перебираем все созданные потоки
            th.join();                                       // Ждём завершения каждого потока
        }
        image = std::move(dst);                              // Заменяем исходное изображение обработанным
    }
    // Вспомогательный метод для подсчёта средней яркости всего изображения
    double averageBrightness() const {
        long long sum = 0;                                   // Сумма всех яркостей пикселей
        long long count = 0;                                 // Количество пикселей
        for (int y = 0; y < height; ++y) {                   // Перебираем строки
            for (int x = 0; x < width; ++x) {                // Перебираем столбцы
                sum += image[y][x];                          // Прибавляем яркость текущего пикселя
                ++count;                                     // Увеличиваем счётчик пикселей
            }
        }
        return count > 0 ? static_cast<double>(sum) / count : 0.0; // Вычисляем среднее значение или 0, если изображение пустое
    }
};

// Главная функция демонстрирует применение фильтра как в однопоточном, так и в многопоточном режимах
int main() {
    const int width = 1000;                                  // Ширина изображения для теста
    const int height = 1000;                                 // Высота изображения для теста
    const int num_threads = 4;                               // Количество потоков для многопоточной обработки
    ImageProcessor processor(width, height);                 // Создаём объект класса ImageProcessor с указанными размерами
    // Сохраняем исходную среднюю яркость
    double initial_avg = processor.averageBrightness();      // Вычисляем среднюю яркость исходного изображения
    // Однопоточная обработка и замер времени
    auto start_single = std::chrono::steady_clock::now();    // Фиксируем время до однопоточной обработки
    processor.applyFilterSingleThread();                     // Применяем фильтр размытия в одном потоке
    auto end_single = std::chrono::steady_clock::now();      // Фиксируем время после обработки
    auto duration_single = std::chrono::duration_cast<std::chrono::milliseconds>(end_single - start_single); // Вычисляем длительность обработки
    double single_avg = processor.averageBrightness();       // Средняя яркость после однопоточной обработки
    // Создаём новый процессор для многопоточного теста, чтобы сравнение было корректным
    ImageProcessor processor_multi(width, height);           // Создаём новый объект для многопоточной обработки
    auto start_multi = std::chrono::steady_clock::now();     // Время перед запуском многопоточной обработки
    processor_multi.applyFilterMultiThread(num_threads);     // Запускаем фильтр в многопоточном режиме
    auto end_multi = std::chrono::steady_clock::now();       // Время после завершения
    auto duration_multi = std::chrono::duration_cast<std::chrono::milliseconds>(end_multi - start_multi); // Длительность многопоточного режима
    double multi_avg = processor_multi.averageBrightness();  // Средняя яркость после многопоточной обработки
    // Выводим статистику пользователю
    std::cout << "Исходная средняя яркость: " << initial_avg << std::endl;          // Печатаем исходную среднюю яркость
    std::cout << "Средняя яркость (однопоточно): " << single_avg << std::endl;     // Печатаем среднюю яркость после однопоточной обработки
    std::cout << "Средняя яркость (многопоточно): " << multi_avg << std::endl;     // Печатаем среднюю яркость после многопоточной обработки
    std::cout << "Время однопоточной обработки (мс): " << duration_single.count() << std::endl; // Время выполнения однопоточного фильтра
    std::cout << "Время многопоточной обработки (мс): " << duration_multi.count() << std::endl; // Время выполнения многопоточного фильтра
    return 0;                                                // Возвращаем 0, что означает успешное завершение программы
}