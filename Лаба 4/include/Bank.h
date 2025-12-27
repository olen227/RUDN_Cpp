// Bank.h - заголовочный файл для класса Bank
// Класс Bank управляет коллекциями клиентов, счетов и транзакций и реализует бизнес-логику банка

#ifndef BANK_H // Предотвращаем повторное включение файла
#define BANK_H // Определяем макрос заголовочного файла

#include <vector>         // Подключаем vector для хранения коллекций
#include <memory>         // Подключаем memory для использования умных указателей
#include "Client.h"      // Подключаем базовый класс Client
#include "PremiumClient.h" // Подключаем класс PremiumClient
#include "BankAccount.h" // Подключаем класс BankAccount
#include "Transaction.h" // Подключаем класс Transaction

// Класс Bank реализует управление клиентами, счетами и транзакциями
class Bank {
private:
    std::vector<std::unique_ptr<Client>> clients; // Вектор умных указателей на клиентов (обычных и премиум)
    std::vector<BankAccount> accounts;            // Вектор банковских счетов
    std::vector<Transaction> transactions;        // Вектор транзакций

    // Приватный метод для поиска клиента по ID
    Client* findClient(int id);
    // Приватный метод для поиска счёта по номеру
    BankAccount* findAccount(int accountNumber);

public:
    // Конструктор по умолчанию (сгенерированный)
    Bank() = default;

    // Методы управления клиентами
    void addClient(std::unique_ptr<Client> client);             // Добавить нового клиента
    bool removeClient(int id);                                  // Удалить клиента по ID (если нет активных счетов)
    Client* getClient(int id);                                  // Получить указатель на клиента по ID
    void viewClients() const;                                   // Вывести список всех клиентов
    void editClient(int id,
                   const std::string &firstName,
                   const std::string &lastName,
                   const Address &address);                     // Изменить данные клиента

    // Методы управления счетами
    BankAccount* openAccount(int clientId,
                            AccountType type,
                            double initialDeposit,
                            const Date &openDate);              // Открыть новый счёт
    bool closeAccount(int accountNumber);                       // Закрыть счёт
    std::vector<BankAccount*> getClientAccounts(int clientId); // Получить список счетов клиента
    void viewAccounts() const;                                  // Вывести список всех счетов

    // Банковские операции
    void deposit(int accountNumber, double amount);             // Пополнить счёт
    void withdraw(int accountNumber, double amount);            // Снять средства со счёта
    void transfer(int fromAccountNumber,
                  int toAccountNumber,
                  double amount);                               // Перевести средства между счетами

    // Отчётность
    void viewTransactions() const;                              // Вывести список всех транзакций
    void printBankInfo() const;                                 // Вывести общую информацию о банке
};

#endif // Завершение условной компиляции