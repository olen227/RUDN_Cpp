// Bank.cpp - файл реализации класса Bank
// Реализует методы для управления клиентами, счетами и транзакциями в банковской системе

#include "Bank.h"        // Подключаем заголовочный файл Bank
#include <iostream>       // Подключаем iostream для вывода на консоль
#include <algorithm>      // Подключаем algorithm для использования std::remove_if

// Приватный метод для поиска клиента по ID
Client* Bank::findClient(int id) {
    for (auto &c : clients) {     // Перебираем всех клиентов в векторе
        if (c->getId() == id) {   // Если ID клиента совпадает с искомым
            return c.get();        // Возвращаем указатель на найденного клиента
        }
    }
    return nullptr;               // Если клиент не найден, возвращаем nullptr
}

// Приватный метод для поиска счёта по номеру
BankAccount* Bank::findAccount(int accountNumber) {
    for (auto &a : accounts) {               // Перебираем все счета
        if (a.getAccountNumber() == accountNumber) { // Если номер счёта совпадает
            return &a;                              // Возвращаем указатель на найденный счёт
        }
    }
    return nullptr;                               // Если счёт не найден, возвращаем nullptr
}

// Добавление клиента в банк
void Bank::addClient(std::unique_ptr<Client> client) {
    clients.push_back(std::move(client)); // Перемещаем переданный умный указатель в вектор клиентов
}

// Удаление клиента по ID (если у клиента нет активных счетов)
bool Bank::removeClient(int id) {
    // Проверяем наличие активных счетов у клиента
    for (auto &a : accounts) {                           // Перебираем все счета
        if (a.getClientId() == id && a.isActive()) {    // Если счёт принадлежит клиенту и активен
            std::cout << "У клиента есть активные счета, удаление невозможно.\n"; // Выводим сообщение об ошибке
            return false;                                // Возвращаем false, т.к. удаление невозможно
        }
    }
    // Удаляем клиента из вектора
    auto it = std::remove_if(clients.begin(), clients.end(), // Используем remove_if для нахождения клиента
                              [id](const std::unique_ptr<Client> &c) { // Лямбда выражение для сравнения ID
                                  return c->getId() == id;            // Возвращаем true, если ID совпадает
                              });
    if (it != clients.end()) {                     // Если клиент найден
        clients.erase(it, clients.end());          // Удаляем клиента из вектора
        return true;                               // Возвращаем true, т.к. удаление успешно
    }
    return false;                                  // Возвращаем false, если клиент не найден
}

// Получить указатель на клиента по ID
Client* Bank::getClient(int id) {
    return findClient(id); // Вызываем приватный метод поиска клиента
}

// Вывод списка всех клиентов
void Bank::viewClients() const {
    if (clients.empty()) {                 // Проверяем, есть ли клиенты
        std::cout << "Нет зарегистрированных клиентов.\n"; // Если нет, выводим сообщение
    } else {
        for (const auto &c : clients) {    // Перебираем всех клиентов
            c->displayInfo();              // Вызываем метод отображения информации
            std::cout << "-----------------------------\n"; // Разделяем клиентов
        }
    }
}

// Редактирование данных клиента
void Bank::editClient(int id,
                      const std::string &firstName,
                      const std::string &lastName,
                      const Address &address) {
    Client *c = findClient(id);            // Ищем клиента по ID
    if (c) {                               // Если клиент найден
        c->setFirstName(firstName);        // Обновляем имя
        c->setLastName(lastName);          // Обновляем фамилию
        c->setAddress(address);            // Обновляем адрес
        std::cout << "Информация о клиенте обновлена.\n"; // Сообщаем об успешном обновлении
    } else {
        std::cout << "Клиент не найден.\n"; // Выводим сообщение, если клиент не найден
    }
}

// Открытие нового счёта для клиента
BankAccount* Bank::openAccount(int clientId,
                               AccountType type,
                               double initialDeposit,
                               const Date &openDate) {
    Client *client = findClient(clientId); // Проверяем, существует ли клиент
    if (!client) {                         // Если клиент не найден
        std::cout << "Клиент не найден.\n"; // Выводим сообщение об ошибке
        return nullptr;                    // Возвращаем nullptr, так как счёт не может быть создан
    }
    BankAccount account(clientId, type, initialDeposit, openDate); // Создаём локальный объект счёта
    accounts.push_back(account);            // Добавляем созданный счёт в вектор счетов
    std::cout << "Счёт успешно открыт.\n"; // Сообщаем об успешном открытии
    return &accounts.back();               // Возвращаем указатель на последний (только что созданный) счёт
}

// Закрытие счёта по номеру
bool Bank::closeAccount(int accountNumber) {
    for (auto &a : accounts) {                   // Перебираем все счета
        if (a.getAccountNumber() == accountNumber) { // Если найден счёт с указанным номером
            if (a.getBalance() != 0.0) {             // Проверяем, что баланс равен нулю
                std::cout << "Для закрытия счёта баланс должен быть нулевым.\n"; // Сообщаем о невозможности закрытия
                return false;                        // Возвращаем false
            }
            if (!a.isActive()) {                    // Проверяем, что счёт уже закрыт
                std::cout << "Счёт уже закрыт.\n"; // Сообщаем, что счёт уже закрыт
                return false;                        // Возвращаем false
            }
            a.deactivate();                          // Деактивируем (закрываем) счёт
            std::cout << "Счёт закрыт.\n";       // Выводим сообщение об успешном закрытии
            return true;                             // Возвращаем true
        }
    }
    std::cout << "Счёт не найден.\n";            // Выводим сообщение, если счёт не найден
    return false;                                    // Возвращаем false
}

// Получение списка счетов клиента по ID клиента
std::vector<BankAccount*> Bank::getClientAccounts(int clientId) {
    std::vector<BankAccount*> results;            // Создаём временный вектор для хранения результатов
    for (auto &a : accounts) {                    // Перебираем все счета
        if (a.getClientId() == clientId) {       // Если счёт принадлежит данному клиенту
            results.push_back(&a);                // Добавляем указатель на счёт в результирующий вектор
        }
    }
    return results;                               // Возвращаем вектор указателей на счета
}

// Вывод списка всех счетов
void Bank::viewAccounts() const {
    if (accounts.empty()) {                       // Проверяем, есть ли счета
        std::cout << "Нет доступных счетов.\n"; // Если нет счетов, выводим сообщение
    } else {
        for (const auto &a : accounts) {          // Перебираем все счета
            a.displayInfo();                      // Выводим информацию о счёте
            std::cout << "-----------------------------\n"; // Разделяем выводы
        }
    }
}

// Пополнение счёта
void Bank::deposit(int accountNumber, double amount) {
    BankAccount *a = findAccount(accountNumber); // Ищем счёт по номеру
    if (a) {                                     // Если счёт найден
        a->deposit(amount);                      // Пополняем счёт
        transactions.emplace_back(accountNumber, accountNumber, amount, TransactionType::Deposit); // Фиксируем транзакцию в истории (from и to одинаковые для пополнения)
        std::cout << "Пополнение выполнено успешно.\n";     // Выводим сообщение об успешном пополнении
    } else {
        std::cout << "Счёт не найден.\n";     // Выводим сообщение, если счёт не найден
    }
}

// Снятие средств со счёта
void Bank::withdraw(int accountNumber, double amount) {
    BankAccount *a = findAccount(accountNumber); // Ищем счёт по номеру
    if (a) {                                     // Если счёт найден
        if (a->withdraw(amount)) {               // Пытаемся снять средства
            transactions.emplace_back(accountNumber, accountNumber, amount, TransactionType::Withdrawal); // Добавляем запись о снятии в историю
            std::cout << "Снятие выполнено успешно.\n"; // Сообщаем об успешной операции
        } else {
            std::cout << "Недостаточно средств.\n";    // Сообщаем о недостатке средств
        }
    } else {
        std::cout << "Счёт не найден.\n";         // Сообщаем, что счёт не найден
    }
}

// Перевод средств между счетами
void Bank::transfer(int fromAccountNumber, int toAccountNumber, double amount) {
    BankAccount *from = findAccount(fromAccountNumber); // Находим счёт отправителя
    BankAccount *to = findAccount(toAccountNumber);     // Находим счёт получателя
    if (!from || !to) {                                 // Проверяем, что оба счёта существуют
        std::cout << "Один или оба счёта не найдены.\n"; // Сообщаем, что один или оба счёта не найдены
        return;                                         // Выходим из метода, так как операция невозможна
    }
    if (!from->isActive() || !to->isActive()) {         // Проверяем, что оба счёта активны
        std::cout << "Один из счетов не активен.\n"; // Сообщаем, что один из счетов не активен
        return;                                         // Выходим из метода
    }
    if (from->transferTo(*to, amount)) {                // Пытаемся выполнить перевод
        transactions.emplace_back(fromAccountNumber, toAccountNumber, amount, TransactionType::Transfer); // Добавляем запись о переводе в историю
        std::cout << "Перевод выполнен успешно.\n";         // Сообщаем об успешном переводе
    } else {
        std::cout << "Недостаточно средств.\n";          // Сообщаем о недостатке средств
    }
}

// Вывод списка всех транзакций
void Bank::viewTransactions() const {
    if (transactions.empty()) {                         // Проверяем, есть ли транзакции
        std::cout << "Нет транзакций.\n";             // Если нет, выводим сообщение
    } else {
        for (const auto &t : transactions) {            // Перебираем все транзакции
            t.displayInfo();                            // Выводим информацию о транзакции
            std::cout << "-----------------------------\n"; // Разделяем выводы
        }
    }
}

// Вывод общей информации о банке
void Bank::printBankInfo() const {
    std::cout << "Информация о банке:\n";                       // Выводим заголовок
    std::cout << "Всего клиентов: " << clients.size() << "\n";      // Выводим общее количество клиентов
    std::cout << "Всего счетов: " << accounts.size() << "\n";    // Выводим общее количество счетов
    std::cout << "Всего транзакций: " << transactions.size() << "\n"; // Выводим общее количество транзакций
}