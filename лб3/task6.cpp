#include <iostream>
#include <string>
using namespace std;

class BankAccount {
private:
    string accountNumber;
    string ownerName;
    double balance;
    static int totalAccounts;           // Общее количество счетов
    static double totalBankBalance;     // Общий баланс банка
    const double MIN_BALANCE = 10.0;    // Минимальный баланс

public:
    // TODO: Создать конструктор с параметрами
    // В конструкторе увеличивать totalAccounts и добавлять баланс к totalBankBalance
    
    // Конструктор класса BankAccount с инициализацией полей и обновлением статических переменных
    BankAccount(string accNum, string owner, double initialBalance) 
        : accountNumber(accNum), ownerName(owner), balance(initialBalance) {
        totalAccounts++;                   // Увеличение общего количества счетов
        totalBankBalance += balance;       // Добавление баланса к общему балансу банка
        cout << "Создан счет " << accountNumber << " для " << ownerName << endl;
    }
    
    // TODO: Создать деструктор
    // В деструкторе уменьшать totalAccounts и вычитать баланс из totalBankBalance
    
    // Деструктор класса BankAccount с обновлением статических переменных
    ~BankAccount() {
        totalAccounts--;                   // Уменьшение общего количества счетов
        totalBankBalance -= balance;       // Вычитание баланса из общего баланса банка
        cout << "Счет " << accountNumber << " закрыт." << endl;
    }
    
    // TODO: Реализовать методы:
    // - deposit(double amount)
    
    // Метод для пополнения счета
    void deposit(double amount) {
        if (amount > 0) {  // Проверка корректности суммы
            balance += amount;                // Увеличение баланса счета
            totalBankBalance += amount;       // Увеличение общего баланса банка
            cout << "Пополнение счета " << accountNumber << " на " << amount << " руб." << endl;
            cout << "Новый баланс: " << balance << " руб." << endl;
        } else {
            cout << "Неверная сумма пополнения." << endl;
        }
    }
    
    // - withdraw(double amount) с проверкой минимального баланса
    
    // Метод для снятия денег со счета с проверкой минимального баланса
    bool withdraw(double amount) {
        if (amount <= 0) {  // Проверка корректности суммы
            cout << "Неверная сумма для снятия." << endl;
            return false;
        }
        
        // Проверка: не опустится ли баланс ниже минимального после снятия
        if (balance - amount < MIN_BALANCE) {
            cout << "Недостаточно средств. Минимальный баланс: " << MIN_BALANCE << " руб." << endl;
            return false;
        }
        
        balance -= amount;                // Уменьшение баланса счета
        totalBankBalance -= amount;       // Уменьшение общего баланса банка
        cout << "Снятие со счета " << accountNumber << ": " << amount << " руб." << endl;
        cout << "Новый баланс: " << balance << " руб." << endl;
        return true;
    }
    
    // - displayAccountInfo() const
    
    // Метод для вывода информации о счете
    void displayAccountInfo() const {
        cout << "=== Информация о счете ===" << endl;
        cout << "Номер счета: " << accountNumber << endl;
        cout << "Владелец: " << ownerName << endl;
        cout << "Баланс: " << balance << " руб." << endl;
        cout << "Минимальный баланс: " << MIN_BALANCE << " руб." << endl;
    }
    
    // TODO: Статические методы:
    // - static int getTotalAccounts()
    
    // Статический метод для получения общего количества счетов
    static int getTotalAccounts() {
        return totalAccounts;
    }
    
    // - static double getTotalBankBalance()
    
    // Статический метод для получения общего баланса всех счетов в банке
    static double getTotalBankBalance() {
        return totalBankBalance;
    }
    
    // - static double getAverageBalance()
    
    // Статический метод для вычисления среднего баланса по всем счетам
    static double getAverageBalance() {
        if (totalAccounts > 0) {  // Проверка деления на ноль
            return totalBankBalance / totalAccounts;  // Среднее = общий баланс / количество счетов
        }
        return 0.0;
    }
};

// TODO: Инициализировать статические переменные

// Инициализация статической переменной для подсчета общего количества счетов
int BankAccount::totalAccounts = 0;

// Инициализация статической переменной для подсчета общего баланса банка
double BankAccount::totalBankBalance = 0.0;

int main() {
    // TODO: Создать несколько счетов
    // Продемонстрировать работу статических методов
    // Показать, что статические переменные общие для всех объектов
    
    cout << "=== Создание счетов ===" << endl;
    // Создание счетов через динамическое выделение памяти
    BankAccount* acc1 = new BankAccount("001", "Иван Иванов", 5000.0);
    BankAccount* acc2 = new BankAccount("002", "Петр Петров", 10000.0);
    BankAccount* acc3 = new BankAccount("003", "Мария Сидорова", 15000.0);
    
    cout << "\n=== Статистика банка ===" << endl;
    // Демонстрация работы статических методов
    cout << "Всего счетов: " << BankAccount::getTotalAccounts() << endl;
    cout << "Общий баланс банка: " << BankAccount::getTotalBankBalance() << " руб." << endl;
    cout << "Средний баланс: " << BankAccount::getAverageBalance() << " руб." << endl;
    
    cout << "\n=== Операции со счетами ===" << endl;
    acc1->displayAccountInfo();
    cout << endl;
    
    // Демонстрация операций со счетом
    acc1->deposit(2000);    // Пополнение счета
    acc1->withdraw(3000);   // Снятие денег
    cout << endl;
    
    acc2->displayAccountInfo();
    cout << endl;
    
    acc2->withdraw(8000);   // Попытка снять много денег
    cout << endl;
    
    cout << "=== Обновленная статистика банка ===" << endl;
    // Вывод обновленной статистики после операций
    cout << "Всего счетов: " << BankAccount::getTotalAccounts() << endl;
    cout << "Общий баланс банка: " << BankAccount::getTotalBankBalance() << " руб." << endl;
    cout << "Средний баланс: " << BankAccount::getAverageBalance() << " руб." << endl;
    
    cout << "\n=== Закрытие счета ===" << endl;
    delete acc2;  // Удаление счета (вызов деструктора)
    
    cout << "\n=== Финальная статистика ===" << endl;
    // Вывод финальной статистики после закрытия одного счета
    cout << "Всего счетов: " << BankAccount::getTotalAccounts() << endl;
    cout << "Общий баланс банка: " << BankAccount::getTotalBankBalance() << " руб." << endl;
    cout << "Средний баланс: " << BankAccount::getAverageBalance() << " руб." << endl;
    
    // Удаление оставшихся счетов
    delete acc1;
    delete acc3;

}