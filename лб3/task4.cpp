#include <iostream>
#include <string>
#include <vector>
using namespace std;

class Vehicle {
protected:
    string brand;
    string model;
    int year;
    double fuelLevel;

public:
    Vehicle(string b, string m, int y) : brand(b), model(m), year(y), fuelLevel(100) {}
    virtual ~Vehicle() {}
    
    // TODO: Сделать класс абстрактным с чисто виртуальными методами:
    // - startEngine()
    // - stopEngine()
    // - refuel(double amount)
    
    // Чисто виртуальный метод для запуска двигателя
    virtual void startEngine() = 0;
    
    // Чисто виртуальный метод для остановки двигателя
    virtual void stopEngine() = 0;
    
    // Чисто виртуальный метод для заправки транспортного средства
    virtual void refuel(double amount) = 0;
    
    virtual void displayInfo() const {
        cout << brand << " " << model << " (" << year << ")" << endl;
        cout << "Уровень топлива: " << fuelLevel << "%" << endl;
    }
    
    double getFuelLevel() const {
        return fuelLevel;
    }

    void setFuelLevel(double level) { 
        if (level >= 0 && level <= 100) fuelLevel = level; 
    }
};

class Car : public Vehicle {
private:
    int doors;
    string transmission;

public:
    Car(string b, string m, int y, int d, string trans) 
        : Vehicle(b, m, y), doors(d), transmission(trans) {}
    
    // TODO: Реализовать чисто виртуальные методы
    
    // Переопределение метода запуска двигателя автомобиля
    void startEngine() override {
        cout << "Автомобиль " << brand << " " << model << ": Двигатель запущен. Врум-врум!" << endl;
    }
    
    // Переопределение метода остановки двигателя автомобиля
    void stopEngine() override {
        cout << "Автомобиль " << brand << " " << model << ": Двигатель заглушен." << endl;
    }
    
    // Переопределение метода заправки автомобиля
    void refuel(double amount) override {
        double newLevel = fuelLevel + amount;  // Вычисление нового уровня топлива
        if (newLevel > 100) {
            newLevel = 100;    // Ограничение максимального уровня до 100%
        }
        fuelLevel = newLevel;                  // Установка нового уровня топлива
        cout << "Заправка автомобиля. Новый уровень топлива: " << fuelLevel << "%" << endl;
    }
    
    // Переопределение метода для вывода информации об автомобиле
    void displayInfo() const override {
        cout << "=== Автомобиль ===" << endl;
        Vehicle::displayInfo();  // Вызов метода базового класса
        cout << "Количество дверей: " << doors << endl;  // Вывод количества дверей
        cout << "Трансмиссия: " << transmission << endl;  // Вывод типа трансмиссии
    }
};

class Motorcycle : public Vehicle {
private:
    bool hasFairing;

public:
    Motorcycle(string b, string m, int y, bool fairing) 
        : Vehicle(b, m, y), hasFairing(fairing) {}
    
    // TODO: Реализовать чисто виртуальные методы
    
    // Переопределение метода запуска двигателя мотоцикла
    void startEngine() override {
        cout << "Мотоцикл " << brand << " " << model << ": ВРУМ-ВРУМ! Двигатель ревёт!" << endl;
    }
    
    // Переопределение метода остановки двигателя мотоцикла
    void stopEngine() override {
        cout << "Мотоцикл " << brand << " " << model << ": Двигатель выключен." << endl;
    }
    
    // Переопределение метода заправки мотоцикла
    void refuel(double amount) override {
        double newLevel = fuelLevel + amount;  // Вычисление нового уровня топлива
        if (newLevel > 100) {
            newLevel = 100;    // Ограничение максимального уровня до 100%
        }
        fuelLevel = newLevel;                  // Установка нового уровня топлива
        cout << "Заправка мотоцикла. Новый уровень топлива: " << fuelLevel << "%" << endl;
    }
    
    // Переопределение метода для вывода информации о мотоцикле
    void displayInfo() const override {
        cout << "=== Мотоцикл ===" << endl;
        Vehicle::displayInfo();  // Вызов метода базового класса
        cout << "Обтекатель: " << (hasFairing ? "Да" : "Нет") << endl;  // Вывод наличия обтекателя
    }
    
    void wheelie() {
        cout << brand << " " << model << " делает вилли!" << endl;
    }
};

int main() {
    // TODO: Создать массив указателей на Vehicle
    // Добавить объекты Car и Motorcycle
    // Продемонстрировать полиморфное поведение
    
    // Создание вектора указателей на базовый класс Vehicle
    vector<Vehicle*> vehicles;
    
    // Добавление объектов производных классов через указатели на базовый класс
    vehicles.push_back(new Car("Toyota", "Camry", 2020, 4, "Автомат"));
    vehicles.push_back(new Motorcycle("Harley-Davidson", "Street 750", 2019, false));
    vehicles.push_back(new Car("BMW", "X5", 2021, 5, "Автомат"));
    
    cout << "=== Демонстрация полиморфизма ===" << endl << endl;
    
    // Цикл для демонстрации полиморфного поведения всех транспортных средств
    for (auto vehicle : vehicles) {
        vehicle->displayInfo();         // Полиморфный вызов displayInfo()
        vehicle->startEngine();         // Полиморфный вызов startEngine()
        vehicle->setFuelLevel(50);      // Установка уровня топлива на 50%
        vehicle->refuel(30);            // Полиморфный вызов refuel()
        vehicle->stopEngine();          // Полиморфный вызов stopEngine()
        cout << endl;
    }
    
    // Демонстрация специфичного метода мотоцикла через dynamic_cast
    Motorcycle* bike = dynamic_cast<Motorcycle*>(vehicles[1]);
    if (bike) {  // Проверка успешности приведения типа
        bike->wheelie();  // Вызов метода, специфичного для мотоцикла
    }
    
    // Очистка динамически выделенной памяти
    for (auto vehicle : vehicles) {
        delete vehicle;
    }
}