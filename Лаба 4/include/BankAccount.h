// BankAccount.h - заголовочный файл для класса BankAccount
// Этот класс моделирует банковский счёт и предоставляет основные операции

#ifndef BANKACCOUNT_H // Директива для предотвращения повторного включения
#define BANKACCOUNT_H // Определяем макрос для условной компиляции

#include "Date.h" // Подключаем заголовочный файл Date для хранения даты открытия счёта

// Перечисление AccountType представляет тип банковского счёта
enum class AccountType {
    Checking, // Расчётный (текущий) счёт
    Savings   // Сберегательный счёт
};

// Класс BankAccount определяет структуру и операции банковского счёта
class BankAccount {
private:
    int accountNumber; // Уникальный номер счёта
    int clientId;      // Идентификатор владельца (клиента)
    double balance;    // Текущий баланс на счёте
    AccountType type;  // Тип счёта (расчётный или сберегательный)
    Date openDate;     // Дата открытия счёта
    bool active;       // Статус активности счёта (true - активен, false - закрыт)
    static int accountCounter; // Статический счётчик для генерации уникальных номеров счётов

public:
    // Конструктор класса. Принимает ID клиента, тип счёта, начальный депозит и дату открытия
    BankAccount(int clientId,
                AccountType type,
                double initialDeposit,
                const Date &openDate);

    // Геттеры для доступа к закрытым полям
    int getAccountNumber() const; // Получить номер счёта
    int getClientId() const;      // Получить ID клиента-владельца
    double getBalance() const;    // Получить текущий баланс
    AccountType getType() const;  // Получить тип счёта
    Date getOpenDate() const;     // Получить дату открытия
    bool isActive() const;        // Проверить, активен ли счёт

    // Методы для операций над счётом
    void deposit(double amount);            // Метод для пополнения счёта на заданную сумму
    bool withdraw(double amount);           // Метод для снятия суммы; возвращает false, если средств недостаточно
    bool transferTo(BankAccount &recipient, // Перевод средств на другой счёт
                    double amount);
    void deactivate();                      // Деактивировать (закрыть) счёт
    void activate();                        // Активировать счёт

    // Статический метод возвращает количество созданных счётов
    static int getAccountCount();
    // Метод для вывода информации о счёте
    void displayInfo() const;
};

#endif // Завершение условной компиляции