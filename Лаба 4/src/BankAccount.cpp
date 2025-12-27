// BankAccount.cpp - файл реализации класса BankAccount
// Содержит определения конструкторов и методов класса BankAccount

#include "BankAccount.h" // Подключаем соответствующий заголовочный файл
#include <iostream>       // Подключаем iostream для вывода информации на консоль

// Инициализация статического счётчика номеров счётов
int BankAccount::accountCounter = 1000; // Начинаем нумерацию счётов с 1001

// Конструктор класса BankAccount
// Создаёт новый счёт с уникальным номером, указанным владельцем, типом, начальным депозитом и датой открытия
BankAccount::BankAccount(int clientId,
                         AccountType type,
                         double initialDeposit,
                         const Date &openDate)
    : accountNumber(++accountCounter), // Генерируем уникальный номер счёта, инкрементируя счётчик
      clientId(clientId),              // Сохраняем идентификатор клиента
      balance(initialDeposit),         // Устанавливаем начальный баланс
      type(type),                      // Устанавливаем тип счёта
      openDate(openDate),              // Сохраняем дату открытия
      active(true) {}                  // По умолчанию счёт активен

// Геттер для номера счёта
int BankAccount::getAccountNumber() const {
    return accountNumber; // Возвращаем уникальный номер счёта
}

// Геттер для ID клиента
int BankAccount::getClientId() const {
    return clientId; // Возвращаем идентификатор клиента, владеющего этим счётом
}

// Геттер для баланса
double BankAccount::getBalance() const {
    return balance; // Возвращаем текущий баланс счёта
}

// Геттер для типа счёта
AccountType BankAccount::getType() const {
    return type; // Возвращаем тип счёта (Checking или Savings)
}

// Геттер для даты открытия счёта
Date BankAccount::getOpenDate() const {
    return openDate; // Возвращаем дату открытия счёта
}

// Метод, проверяющий, активен ли счёт
bool BankAccount::isActive() const {
    return active; // Возвращаем текущий статус активности
}

// Метод для пополнения счёта
void BankAccount::deposit(double amount) {
    if (amount > 0) {          // Проверяем, что сумма положительная
        balance += amount;     // Увеличиваем баланс на указанную сумму
    }
}

// Метод для снятия средств со счёта
bool BankAccount::withdraw(double amount) {
    if (amount > 0 &&          // Проверяем, что сумма положительная
        balance >= amount) {   // Проверяем, что на счёте достаточно средств
        balance -= amount;     // Уменьшаем баланс на указанную сумму
        return true;           // Возвращаем true, если снятие успешно
    }
    return false;             // Возвращаем false, если средств недостаточно или сумма некорректна
}

// Метод для перевода средств на другой счёт
bool BankAccount::transferTo(BankAccount &recipient, double amount) {
    if (this->withdraw(amount)) { // Пытаемся снять указанную сумму с текущего счёта
        recipient.deposit(amount); // Если успешно, пополняем счёт получателя
        return true;               // Возвращаем true, если операция завершилась успешно
    }
    return false;                 // Возвращаем false, если на счёте недостаточно средств
}

// Метод деактивации счёта
void BankAccount::deactivate() {
    active = false; // Устанавливаем статус активности в false, что означает закрытие счёта
}

// Метод активации счёта
void BankAccount::activate() {
    active = true; // Устанавливаем статус активности в true, что означает открытие счёта
}

// Статический метод для получения количества созданных счётов
int BankAccount::getAccountCount() {
    return accountCounter - 1000; // Вычисляем количество счётов, вычитая стартовое значение счётчика
}

// Метод для отображения информации о счёте
void BankAccount::displayInfo() const {
    std::cout << "Номер счёта: " << accountNumber << "\n";                        // Выводим номер счёта
    std::cout << "ID клиента: " << clientId << "\n";                                  // Выводим ID клиента
    std::cout << "Баланс: " << balance << "\n";                                      // Выводим текущий баланс
    std::cout << "Тип: " << (type == AccountType::Checking ? "Расчётный" : "Сберегательный") // Определяем тип счёта и выводим его
              << "\n";
    std::cout << "Дата открытия: " << openDate.toString() << "\n";                       // Выводим дату открытия счёта
    std::cout << "Статус: " << (active ? "Активен" : "Неактивен") << "\n";              // Выводим статус активности счёта
}