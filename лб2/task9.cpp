#include <iostream>
#include <iomanip>
using namespace std;

struct Account {
    int accountNumber;
    double balance;
};

void deposit(Account* acc, double amount) {
    if (acc == nullptr) {
        cerr << "Ошибка: указатель на счет равен nullptr\n";
        return;
    }
    
    if (amount <= 0) {
        cerr << "Ошибка: сумма пополнения должна быть положительной\n";
        return;
    }
    
    if (amount > 1000000) {
        cerr << "Предупреждение: очень большая сумма пополнения\n";
    }
    
    acc->balance += amount;
    cout << "Счет " << acc->accountNumber 
         << " пополнен на " << fixed << setprecision(2) << amount << " руб.\n";
}

void withdraw(Account& acc, double amount) {
    if (amount <= 0) {
        cerr << "Ошибка: сумма снятия должна быть положительной\n";
        return;
    }
    
    if (acc.balance < amount) {
        cerr << "Ошибка: недостаточно средств на счете\n";
        cerr << "Запрошено: " << fixed << setprecision(2) << amount << " руб.\n";
        cerr << "Доступно: " << acc.balance << " руб.\n";
        return;
    }
    
    acc.balance -= amount;
    cout << "Со счета " << acc.accountNumber 
         << " снято " << fixed << setprecision(2) << amount << " руб.\n";
}

void printAccount(const Account& acc) {
    cout << "\n--- Информация о счете ---\n";
    cout << "Номер счета: " << acc.accountNumber << "\n";
    cout << "Баланс: " << fixed << setprecision(2) << acc.balance << " руб.\n";
}

int main() {
    cout << "=== БАНКОВСКАЯ СИСТЕМА ===\n";
    
    Account myAccount = {123456, 10000.00};
    
    cout << "\nИсходное состояние:";
    printAccount(myAccount);
    
    cout << "\n--- ПОПОЛНЕНИЕ СЧЕТА (через указатель) ---\n";
    deposit(&myAccount, 5000.00);
    printAccount(myAccount);
    
    cout << "\n--- СНЯТИЕ СРЕДСТВ (через ссылку) ---\n";
    withdraw(myAccount, 3000.00);
    printAccount(myAccount);
    
    cout << "\n--- ЕЩЕ ОПЕРАЦИИ ---\n";
    deposit(&myAccount, 2500.00);
    withdraw(myAccount, 1000.00);
    printAccount(myAccount);
    
    cout << "\n=== ТЕСТИРОВАНИЕ ЗАЩИТЫ ===\n";
    
    cout << "\n1. Попытка снять больше, чем есть на счете:\n";
    withdraw(myAccount, 100000.00);
    printAccount(myAccount);
    
    cout << "\n2. Попытка пополнить на отрицательную сумму:\n";
    deposit(&myAccount, -500.00);
    
    cout << "\n3. Попытка снять отрицательную сумму:\n";
    withdraw(myAccount, -100.00);
    
    cout << "\n4. Попытка работы с nullptr:\n";
    deposit(nullptr, 1000.00);
    
    cout << "\n5. Очень большая сумма:\n";
    deposit(&myAccount, 5000000.00);
    
    cout << "\nИтоговое состояние:";
    printAccount(myAccount);
    
    cout << "\n--- РАЗНИЦА МЕЖДУ УКАЗАТЕЛЕМ И ССЫЛКОЙ ---\n";
    cout << "Указатель: deposit(&acc, sum)  ->  acc->balance\n";
    cout << "Ссылка:    withdraw(acc, sum)  ->  acc.balance\n";
    cout << "Оба способа изменяют оригинальные данные\n";
    
    return 0;
}
