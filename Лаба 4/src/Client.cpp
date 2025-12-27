// Client.cpp - файл реализации класса Client
// Содержит определения конструкторов, деструктора и методов класса Client

#include "Client.h"    // Подключаем заголовочный файл Client
#include <iostream>     // Подключаем iostream для вывода на консоль

// Инициализация статического счётчика клиентов
int Client::clientCounter = 0; // Начальное значение для счётчика клиентов

// Конструктор по умолчанию. Присваивает уникальный ID и дефолтные значения полям
Client::Client()
    : id(++clientCounter),    // Инкрементируем счётчик и присваиваем его значение идентификатору
      firstName("Unknown"),   // Устанавливаем имя по умолчанию
      lastName("Unknown"),    // Устанавливаем фамилию по умолчанию
      address(),               // Вызываем конструктор по умолчанию для адреса
      registrationDate() {}    // Вызываем конструктор по умолчанию для даты регистрации

// Параметризованный конструктор. Принимает имя, фамилию, адрес и дату регистрации
Client::Client(const std::string &firstName,
               const std::string &lastName,
               const Address &address,
               const Date &registrationDate)
    : id(++clientCounter),        // Генерируем уникальный идентификатор клиента
      firstName(firstName),       // Инициализируем поле firstName переданным значением
      lastName(lastName),         // Инициализируем поле lastName переданным значением
      address(address),           // Инициализируем поле address копией переданного адреса
      registrationDate(registrationDate) {} // Инициализируем поле registrationDate копией переданной даты

// Виртуальный деструктор
Client::~Client() {
    // Тело деструктора может быть пустым, так как динамическая память не используется напрямую
}

// Геттер для идентификатора клиента
int Client::getId() const {
    return id; // Возвращаем идентификатор
}

// Геттер для имени клиента
std::string Client::getFirstName() const {
    return firstName; // Возвращаем имя клиента
}

// Геттер для фамилии клиента
std::string Client::getLastName() const {
    return lastName; // Возвращаем фамилию клиента
}

// Геттер для адреса клиента
Address Client::getAddress() const {
    return address; // Возвращаем копию адреса клиента
}

// Геттер для даты регистрации клиента
Date Client::getRegistrationDate() const {
    return registrationDate; // Возвращаем копию даты регистрации
}

// Сеттер для имени клиента
void Client::setFirstName(const std::string &firstName) {
    this->firstName = firstName; // Присваиваем новое значение полю firstName
}

// Сеттер для фамилии клиента
void Client::setLastName(const std::string &lastName) {
    this->lastName = lastName; // Присваиваем новое значение полю lastName
}

// Сеттер для адреса клиента
void Client::setAddress(const Address &address) {
    this->address = address; // Присваиваем новое значение полю address
}

// Сеттер для даты регистрации клиента
void Client::setRegistrationDate(const Date &registrationDate) {
    this->registrationDate = registrationDate; // Присваиваем новое значение полю registrationDate
}

// Статический метод для получения количества созданных клиентов
int Client::getClientCount() {
    return clientCounter; // Возвращаем текущее значение статического счётчика
}

// Метод для отображения информации о клиенте
void Client::displayInfo() const {
    // Выводим данные клиента в читаемом виде
    std::cout << "ID клиента: " << id << "\n";                                // Выводим идентификатор клиента
    std::cout << "Имя: " << firstName << " " << lastName << "\n";            // Выводим имя и фамилию клиента
    std::cout << "Адрес: " << address.streetAndHouse << ", "               // Выводим улицу и дом
              << address.city << ", " << address.postalIndex << ", "         // Выводим город и почтовый индекс
              << address.country << "\n";                                      // Выводим страну
    std::cout << "Дата регистрации: " << registrationDate.toString() << "\n"; // Выводим дату регистрации клиента
}