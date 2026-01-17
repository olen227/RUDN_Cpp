// Address.cpp - файл реализации структуры Address
// Этот файл содержит определения конструкторов структуры Address

#include "Address.h" // Подключаем соответствующий заголовочный файл

// Конструктор по умолчанию. Инициализирует все поля пустыми строками
Address::Address()
    : streetAndHouse(""), // инициализируем поле streetAndHouse пустой строкой
      city(""),            // инициализируем поле city пустой строкой
      postalIndex(""),     // инициализируем поле postalIndex пустой строкой
      country("") {}        // инициализируем поле country пустой строкой

// Параметризованный конструктор. Принимает значения для всех полей
Address::Address(const std::string &streetAndHouse,
                 const std::string &city,
                 const std::string &postalIndex,
                 const std::string &country)
    : streetAndHouse(streetAndHouse), // инициализируем поле streetAndHouse переданным значением
      city(city),                     // инициализируем поле city переданным значением
      postalIndex(postalIndex),       // инициализируем поле postalIndex переданным значением
      country(country) {}             // инициализируем поле country переданным значением