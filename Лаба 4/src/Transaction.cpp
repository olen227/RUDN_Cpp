// Transaction.cpp - файл реализации класса Transaction
// Здесь реализованы конструктор и методы класса Transaction

#include "Transaction.h" // Подключаем заголовочный файл Transaction
#include <iostream>       // Подключаем iostream для вывода на консоль
#include <iomanip>        // Подключаем iomanip для форматирования вывода
#include <sstream>        // Подключаем sstream для использования ostringstream

// Инициализация статического счётчика транзакций
int Transaction::transactionCounter = 0; // Счётчик начинается с нуля

// Конструктор класса Transaction
// Принимает номера счётов (fromAccount, toAccount), сумму и тип операции
Transaction::Transaction(int fromAccount,
                         int toAccount,
                         double amount,
                         TransactionType type)
    : id(++transactionCounter),   // Генерируем уникальный ID, инкрементируя счётчик
      fromAccount(fromAccount),   // Сохраняем номер счёта отправителя
      toAccount(toAccount),       // Сохраняем номер счёта получателя
      amount(amount),             // Сохраняем сумму операции
      type(type) {                // Сохраняем тип транзакции
    // Получаем текущее системное время
    std::time_t now = std::time(nullptr);      // Получаем текущее время в секундах
    std::tm *ltm = std::localtime(&now);       // Конвертируем в локальное время
    std::ostringstream oss;                    // Создаём строковый поток для форматирования даты и времени
    oss << std::put_time(ltm, "%d.%m.%Y %H:%M:%S"); // Форматируем дату и время в строку
    timestamp = oss.str();                     // Сохраняем результат в поле timestamp
}

// Геттер для ID транзакции
int Transaction::getId() const {
    return id; // Возвращаем уникальный идентификатор транзакции
}

// Геттер для счёта отправителя
int Transaction::getFromAccount() const {
    return fromAccount; // Возвращаем номер счёта отправителя
}

// Геттер для счёта получателя
int Transaction::getToAccount() const {
    return toAccount; // Возвращаем номер счёта получателя
}

// Геттер для суммы операции
double Transaction::getAmount() const {
    return amount; // Возвращаем сумму операции
}

// Геттер для временной метки
std::string Transaction::getTimestamp() const {
    return timestamp; // Возвращаем строковое представление временной метки
}

// Геттер для типа транзакции
TransactionType Transaction::getType() const {
    return type; // Возвращаем тип транзакции (Deposit, Withdrawal, Transfer)
}

// Метод для отображения информации о транзакции
void Transaction::displayInfo() const {
    // Выводим ID транзакции
    std::cout << "ID транзакции: " << id << "\n";
    // Выводим тип операции в виде строки
    std::cout << "Type: ";
    switch (type) {                                  // Определяем строковое значение типа
        case TransactionType::Deposit:               // Если операция - пополнение
            std::cout << "Пополнение";                 // Выводим "Deposit"
            break;                                   // Выходим из switch
        case TransactionType::Withdrawal:            // Если операция - снятие
            std::cout << "Снятие";              // Выводим "Withdrawal"
            break;                                   // Выходим из switch
        case TransactionType::Transfer:              // Если операция - перевод
            std::cout << "Перевод";                // Выводим "Transfer"
            break;                                   // Выходим из switch
    }
    std::cout << "\n";                             // Переходим на новую строку
    // Выводим номера счётов отправителя и получателя
    std::cout << "Со счёта: " << fromAccount << "\n"; // Выводим счёт отправителя
    std::cout << "На счёт: " << toAccount << "\n";     // Выводим счёт получателя
    // Выводим сумму операции
    std::cout << "Сумма: " << amount << "\n";             // Выводим сумму
    // Выводим временную метку операции
    std::cout << "Дата и время: " << timestamp << "\n";        // Выводим дату и время транзакции
}

// Статический метод для получения количества транзакций
int Transaction::getTransactionCount() {
    return transactionCounter; // Возвращаем текущее значение счётчика транзакций
}