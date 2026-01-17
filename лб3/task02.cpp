#include <iostream>  // Ввод-вывод
#include <string>    // Работа со строками
using namespace std;

// Базовый класс для представления человека
class Person {
protected:
    string name;     // Имя
    int age;         // Возраст
    string address;  // Адрес

public:
    Person(string n, int a, string addr) : name(n), age(a), address(addr) {}
    
    virtual void displayInfo() const {
        cout << "Имя: " << name << ", Возраст: " << age << ", Адрес: " << address << endl;
    }
    
    virtual ~Person() {}
};

// Класс студента, наследуется от Person
class Student : public Person {
private:
    string studentId;      // Идентификатор студента
    double averageGrade;   // Средний балл

public:
    Student(string n, int a, string addr, string id, double grade)
        : Person(n, a, addr), studentId(id), averageGrade(grade) {}
    
    // Переопределение метода вывода информации
    void displayInfo() const override {
        cout << "=== Студент ===" << endl;
        Person::displayInfo();
        cout << "ID студента: " << studentId << endl;
        cout << "Средний балл: " << averageGrade << endl;
    }
    
    // Геттеры
    string getStudentId() const {
        return studentId;
    }
    double getAverageGrade() const {
        return averageGrade;
    }
    
    // Сеттер с валидацией (оценка от 0.0 до 5.0)
    void setAverageGrade(double grade) {
        if (grade >= 0.0 && grade <= 5.0) {
            averageGrade = grade;
        }
    }
};

// Класс профессора, наследуется от Person
class Professor : public Person {
private:
    string department;       // Кафедра
    double salary;           // Базовая зарплата
    int yearsOfExperience;   // Стаж работы (лет)

public:
    Professor(string n, int a, string addr, string dept, double sal, int years)
        : Person(n, a, addr), department(dept), salary(sal), yearsOfExperience(years) {}
    
    // Переопределение метода вывода информации
    void displayInfo() const override {
        cout << "=== Профессор ===" << endl;
        Person::displayInfo();
        cout << "Кафедра: " << department << endl;
        cout << "Зарплата: " << salary << " руб." << endl;
        cout << "Стаж: " << yearsOfExperience << " лет" << endl;
        cout << "Надбавка за стаж: " << calculateExperienceBonus() << " руб." << endl;
    }
    
    // Расчет надбавки за стаж (5% от зарплаты за каждые 5 лет)
    double calculateExperienceBonus() const {
        return salary * 0.05 * (yearsOfExperience / 5);
    }
};

int main() {
    // Создание и тестирование объектов иерархии
    Student student("Петр Петров", 20, "ул. Студенческая, 15", "S12345", 4.3);
    Professor prof("Доктор Иванов", 45, "ул. Академическая, 10", "Компьютерные науки", 50000, 15);
    
    student.displayInfo();
    cout << endl;
    prof.displayInfo();
}