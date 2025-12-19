#include <iostream>   // Библиотека для ввода-вывода
#include <vector>     // Библиотека для работы с динамическими массивами
#include <stdexcept>  // Библиотека для работы с исключениями
using namespace std;

// Шаблонный класс Stack - реализация стека по принципу LIFO (Last In First Out)
template<typename T>
class Stack {
private:
    vector<T> elements;  // Вектор для хранения элементов стека
    int capacity;        // Максимальная вместимость стека

public:
    // Конструктор с параметром размера (по умолчанию 10)
    Stack(int size = 10) : capacity(size) {}
    
    void push(const T& element) {
        if (isFull()) {
            throw overflow_error("Стек переполнен!");
        }
        elements.push_back(element);  // Добавление в конец вектора (вершина стека)
    }
    
    T pop() {
        if (isEmpty()) {
            throw underflow_error("Стек пуст!");
        }
        T element = elements.back();
        elements.pop_back();
        return element;
    }
    
    T top() const {
        if (isEmpty()) {
            throw underflow_error("Стек пуст!");
        }
        return elements.back();
    }
    
    bool isEmpty() const {
        return elements.empty();
    }
    
    bool isFull() const {
        return elements.size() >= capacity;
    }
    
    int size() const {
        return elements.size();
    }
    
    // Метод для вывода содержимого стека на экран
    void display() const {
        cout << "Стек: ";
        for (const auto& elem : elements) {  // Перебор всех элементов стека
            cout << elem << " ";
        }
        cout << endl;
    }
};

// Шаблонный класс Queue - реализация очереди по принципу FIFO (First In First Out)
template<typename T>
class Queue {
private:
    vector<T> elements;  // Вектор для хранения элементов очереди
    int capacity;        // Максимальная вместимость очереди

public:
    // Конструктор с параметром размера (по умолчанию 10)
    Queue(int size = 10) : capacity(size) {}
    
    void enqueue(const T& element) {
        if (isFull()) {
            throw overflow_error("Очередь переполнена!");
        }
        elements.push_back(element);  // Добавление в конец очереди
    }
    
    T dequeue() {
        if (isEmpty()) {
            throw underflow_error("Очередь пуста!");
        }
        T element = elements.front();
        elements.erase(elements.begin());  // Удаление первого элемента
        return element;
    }
    
    T front() const {
        if (isEmpty()) {
            throw underflow_error("Очередь пуста!");
        }
        return elements.front();
    }
    
    bool isEmpty() const {
        return elements.empty();
    }
    
    bool isFull() const {
        return elements.size() >= capacity;
    }
    
    int size() const {
        return elements.size();
    }
    
    // Метод для вывода содержимого очереди на экран
    void display() const {
        cout << "Очередь: ";
        for (const auto& elem : elements) {  // Перебор всех элементов очереди
            cout << elem << " ";
        }
        cout << endl;
    }
};

int main() {
    // === ТЕСТИРОВАНИЕ СТЕКА С ЦЕЛЫМИ ЧИСЛАМИ ===
    cout << "=== Тестирование Stack<int> ===" << endl;
    Stack<int> intStack(5);
    
    intStack.push(10);
    intStack.push(20);
    intStack.push(30);
    intStack.push(40);
    intStack.display();
    
    cout << "Верхний элемент: " << intStack.top() << endl;
    cout << "Размер: " << intStack.size() << endl;
    
    cout << "Извлекаем: " << intStack.pop() << endl;
    cout << "Извлекаем: " << intStack.pop() << endl;
    intStack.display();
    cout << endl;
    
    // === ТЕСТИРОВАНИЕ СТЕКА С ЧИСЛАМИ С ПЛАВАЮЩЕЙ ТОЧКОЙ ===
    cout << "=== Тестирование Stack<double> ===" << endl;
    Stack<double> doubleStack(5);
    
    doubleStack.push(3.14);
    doubleStack.push(2.71);
    doubleStack.push(1.41);
    doubleStack.display();
    
    cout << "Верхний элемент: " << doubleStack.top() << endl;
    cout << "Извлекаем: " << doubleStack.pop() << endl;
    doubleStack.display();
    cout << endl;
    
    // === ТЕСТИРОВАНИЕ СТЕКА СО СТРОКАМИ ===
    cout << "=== Тестирование Stack<string> ===" << endl;
    Stack<string> stringStack(5);
    
    stringStack.push("Hello");
    stringStack.push("World");
    stringStack.push("C++");
    stringStack.display();
    
    cout << "Верхний элемент: " << stringStack.top() << endl;
    cout << "Извлекаем: " << stringStack.pop() << endl;
    stringStack.display();
    cout << endl;
    
    // === ТЕСТИРОВАНИЕ ОЧЕРЕДИ С ЦЕЛЫМИ ЧИСЛАМИ ===
    cout << "=== Тестирование Queue<int> ===" << endl;
    Queue<int> intQueue(5);
    
    intQueue.enqueue(100);
    intQueue.enqueue(200);
    intQueue.enqueue(300);
    intQueue.enqueue(400);
    intQueue.display();
    
    cout << "Первый элемент: " << intQueue.front() << endl;
    cout << "Размер: " << intQueue.size() << endl;
    
    cout << "Извлекаем: " << intQueue.dequeue() << endl;
    cout << "Извлекаем: " << intQueue.dequeue() << endl;
    intQueue.display();
    cout << endl;
    
    // === ТЕСТИРОВАНИЕ ОЧЕРЕДИ СО СТРОКАМИ ===
    cout << "=== Тестирование Queue<string> ===" << endl;
    Queue<string> stringQueue(5);
    
    stringQueue.enqueue("First");
    stringQueue.enqueue("Second");
    stringQueue.enqueue("Third");
    stringQueue.display();
    
    cout << "Первый элемент: " << stringQueue.front() << endl;
    cout << "Извлекаем: " << stringQueue.dequeue() << endl;
    stringQueue.display();
    cout << endl;
    
    // === ТЕСТИРОВАНИЕ ОБРАБОТКИ ПЕРЕПОЛНЕНИЯ ===
    cout << "=== Тестирование переполнения ===" << endl;
    try {
        Stack<int> smallStack(2);
        smallStack.push(1);
        smallStack.push(2);
        cout << "Стек заполнен. Попытка добавить еще..." << endl;
        smallStack.push(3);  // Генерация исключения overflow_error
    } catch (overflow_error& e) {
        cout << "Исключение: " << e.what() << endl;
    }
    
    cout << endl;
    
    // === ТЕСТИРОВАНИЕ ИЗВЛЕЧЕНИЯ ИЗ ПУСТОЙ СТРУКТУРЫ ===
    cout << "=== Тестирование извлечения из пустой структуры ===" << endl;
    try {
        Queue<int> emptyQueue(5);
        cout << "Попытка извлечь из пустой очереди..." << endl;
        emptyQueue.dequeue();  // Генерация исключения underflow_error
    } catch (underflow_error& e) {
        cout << "Исключение: " << e.what() << endl;
    }

}