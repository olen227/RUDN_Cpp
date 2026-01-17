// PremiumClient.cpp - файл реализации класса PremiumClient
// Этот файл содержит определения конструкторов, сеттеров, геттеров и метода displayInfo для премиум-клиента

#include "PremiumClient.h" // Подключаем заголовочный файл PremiumClient
#include <iostream>        // Подключаем iostream для вывода на консоль

// Конструктор по умолчанию. Вызывает конструктор базового класса Client и устанавливает level и discount
PremiumClient::PremiumClient()
    : Client(),        // Вызываем конструктор по умолчанию базового класса Client
      level(1),        // Устанавливаем уровень премиум-статуса по умолчанию (1)
      discount(0.0) {} // Устанавливаем процент скидки по умолчанию (0.0)

// Параметризованный конструктор. Принимает параметры для базового класса и свои собственные поля
PremiumClient::PremiumClient(const std::string &firstName,
                             const std::string &lastName,
                             const Address &address,
                             const Date &registrationDate,
                             int level,
                             double discount)
    : Client(firstName, lastName, address, registrationDate), // Вызываем конструктор базового класса Client с параметрами
      level(level),         // Инициализируем поле level переданным значением
      discount(discount) {} // Инициализируем поле discount переданным значением

// Сеттер для уровня премиум-статуса
void PremiumClient::setLevel(int level) {
    this->level = level; // Присваиваем новое значение уровню премиум-статуса
}

// Сеттер для процента скидки
void PremiumClient::setDiscount(double discount) {
    this->discount = discount; // Присваиваем новое значение проценту скидки
}

// Геттер для уровня премиум-статуса
int PremiumClient::getLevel() const {
    return level; // Возвращаем текущий уровень премиум-статуса
}

// Геттер для процента скидки
double PremiumClient::getDiscount() const {
    return discount; // Возвращаем текущий процент скидки
}

// Переопределённый метод отображения информации о премиум-клиенте
void PremiumClient::displayInfo() const {
    Client::displayInfo();                         // Вызываем метод отображения информации базового класса
    std::cout << "Премиум-уровень: " << level << "\n";           // Выводим уровень премиум-статуса
    std::cout << "Скидка: " << discount * 100 << "%\n";    // Выводим процент скидки в виде процента
}