// Client.h - заголовочный файл для базового класса Client
// Этот класс представляет клиента банка и содержит основные данные и методы

#ifndef CLIENT_H // Предотвращаем повторное включение файла
#define CLIENT_H // Определяем макрос для заголовочного файла

#include <string>   // Используем std::string для хранения строковых данных
#include "Address.h" // Подключаем структуру Address для хранения адреса клиента
#include "Date.h"    // Подключаем структуру Date для хранения даты регистрации клиента

// Класс Client описывает базового клиента банка
class Client {
protected:
    int id;              // Уникальный идентификатор клиента
    std::string firstName; // Имя клиента
    std::string lastName;  // Фамилия клиента
    Address address;       // Адрес клиента (структура Address)
    Date registrationDate; // Дата регистрации клиента
    static int clientCounter; // Статический счётчик для генерации уникальных идентификаторов

public:
    // Конструктор по умолчанию. Создаёт клиента с дефолтными значениями полей
    Client();
    // Параметризованный конструктор. Принимает имя, фамилию, адрес и дату регистрации
    Client(const std::string &firstName,
           const std::string &lastName,
           const Address &address,
           const Date &registrationDate);
    // Виртуальный деструктор. Позволяет корректно удалять объекты через указатель на базовый класс
    virtual ~Client();

    // Геттеры для доступа к полям класса
    int getId() const;              // Получить идентификатор клиента
    std::string getFirstName() const; // Получить имя клиента
    std::string getLastName() const;  // Получить фамилию клиента
    Address getAddress() const;        // Получить адрес клиента
    Date getRegistrationDate() const;  // Получить дату регистрации клиента

    // Сеттеры для изменения значений полей
    void setFirstName(const std::string &firstName); // Установить имя клиента
    void setLastName(const std::string &lastName);    // Установить фамилию клиента
    void setAddress(const Address &address);         // Установить адрес клиента
    void setRegistrationDate(const Date &registrationDate); // Установить дату регистрации

    // Статический метод для получения количества созданных клиентов
    static int getClientCount();

    // Виртуальный метод для отображения информации о клиенте
    virtual void displayInfo() const;
};

#endif // Завершение директивы предотвращения двойного включения