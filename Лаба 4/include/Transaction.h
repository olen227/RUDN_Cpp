// Transaction.h - заголовочный файл для класса Transaction
// Класс Transaction описывает банковскую транзакцию (операцию)

#ifndef TRANSACTION_H // Предотвращаем повторное включение
#define TRANSACTION_H // Определяем макрос для заголовочного файла

#include <string>    // Подключаем string для строковых данных
#include <chrono>    // Подключаем chrono для получения текущего времени
#include <ctime>     // Подключаем ctime для преобразования времени

// Перечисление TransactionType описывает тип банковской операции
enum class TransactionType {
    Deposit,    // Операция пополнения счёта
    Withdrawal, // Операция снятия средств
    Transfer    // Операция перевода средств
};

// Класс Transaction представляет банковскую транзакцию
class Transaction {
private:
    int id;           // Уникальный идентификатор транзакции
    int fromAccount;  // Номер счёта отправителя (для пополнения и снятия совпадает с toAccount)
    int toAccount;    // Номер счёта получателя
    double amount;    // Сумма операции
    std::string timestamp; // Временная метка, когда операция была произведена
    TransactionType type;  // Тип операции
    static int transactionCounter; // Статический счётчик для генерации уникальных ID транзакций

public:
    // Конструктор. Принимает счета отправителя и получателя, сумму и тип операции
    Transaction(int fromAccount,
                int toAccount,
                double amount,
                TransactionType type);
    // Геттер для ID транзакции
    int getId() const;
    // Геттер для счёта отправителя
    int getFromAccount() const;
    // Геттер для счёта получателя
    int getToAccount() const;
    // Геттер для суммы операции
    double getAmount() const;
    // Геттер для временной метки
    std::string getTimestamp() const;
    // Геттер для типа операции
    TransactionType getType() const;
    // Метод для отображения информации о транзакции
    void displayInfo() const;
    // Статический метод для получения количества транзакций
    static int getTransactionCount();
};

#endif // Завершение директивы предотвращения повторного включения