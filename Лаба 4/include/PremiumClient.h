// PremiumClient.h - заголовочный файл для класса PremiumClient
// Этот класс наследуется от Client и добавляет функциональность премиум-клиента

#ifndef PREMIUMCLIENT_H // Предотвращаем повторное включение файла
#define PREMIUMCLIENT_H // Определяем макрос для заголовочного файла

#include "Client.h" // Подключаем базовый класс Client

// Класс PremiumClient представляет премиум-клиента банка
class PremiumClient : public Client {
private:
    int level;       // Уровень премиум-статуса
    double discount; // Процент скидки для премиум-клиента (значение от 0 до 1)

public:
    // Конструктор по умолчанию. Вызывает конструктор базового класса и задаёт значения полей level и discount по умолчанию
    PremiumClient();
    // Параметризованный конструктор. Принимает имя, фамилию, адрес, дату регистрации, уровень и скидку
    PremiumClient(const std::string &firstName,
                  const std::string &lastName,
                  const Address &address,
                  const Date &registrationDate,
                  int level,
                  double discount);
    // Сеттер для уровня премиум-статуса
    void setLevel(int level);
    // Сеттер для процента скидки
    void setDiscount(double discount);
    // Геттер для уровня премиум-статуса
    int getLevel() const;
    // Геттер для процента скидки
    double getDiscount() const;
    // Переопределённый метод отображения информации о клиенте
    void displayInfo() const override;
};

#endif // Завершение директивы предотвращения двойного включения