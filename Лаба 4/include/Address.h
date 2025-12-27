// Address.h - заголовочный файл для структуры Address
// Этот файл объявляет структуру Address, которая используется для хранения информации об адресе клиента
#ifndef ADDRESS_H // Директива препроцессора для предотвращения двойного включения
#define ADDRESS_H // Определяем макрос для условной компиляции

#include <string> // Подключаем заголовок string, так как будем использовать std::string

// Структура Address хранит информацию об адресе клиента
struct Address {
    std::string streetAndHouse; // улица и номер дома
    std::string city;           // город
    std::string postalIndex;    // почтовый индекс
    std::string country;        // страна

    // Конструктор по умолчанию, инициализирует поля пустыми строками
    Address();
    // Параметризованный конструктор, инициализирует поля указанными значениями
    Address(const std::string &streetAndHouse,
            const std::string &city,
            const std::string &postalIndex,
            const std::string &country);
};

#endif // Завершаем условную компиляцию