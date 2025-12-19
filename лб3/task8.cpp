#include <iostream>
#include <string>
using namespace std;

class Person {
protected:
    string name;
    int age;

public:
    Person(string n, int a) : name(n), age(a) {}
    virtual void display() const {
        cout << "Имя: " << name << ", Возраст: " << age << endl;
    }
};

class Employee {
protected:
    string position;
    double salary;

public:
    Employee(string pos, double sal) : position(pos), salary(sal) {}
    virtual void work() const {
        cout << "Выполняет работу как сотрудник" << endl;
    }
};

// TODO: Создать класс Teacher, наследующий от Person и Employee
class Teacher : public Person, public Employee {
protected:
    string subject;
    int experienceYears;

public:
    // TODO: Создать конструктор
    
    // Конструктор класса Teacher с множественным наследованием от Person и Employee
    Teacher(string n, int a, string pos, double sal, string subj, int exp)
        : Person(n, a), Employee(pos, sal), subject(subj), experienceYears(exp) {}
    
    // TODO: Переопределить методы display() и work()
    
    // Переопределение метода display() для вывода информации об учителе
    void display() const override {
        cout << "=== Учитель ===" << endl;
        Person::display();  // Вызов метода базового класса Person
        cout << "Должность: " << position << endl;  // Доступ к полю из класса Employee
        cout << "Зарплата: " << salary << " руб." << endl;
        cout << "Предмет: " << subject << endl;
        cout << "Стаж: " << experienceYears << " лет" << endl;
    }
    
    // Переопределение метода work() для описания работы учителя
    void work() const override {
        cout << "Преподает предмет: " << subject << endl;
    }
    
    // TODO: Добавить специфичные методы для Teacher
    
    // Метод для проведения урока
    void conductLesson() const {
        cout << name << " проводит урок по предмету " << subject << endl;
    }
    
    // Метод для оценивания студентов
    void gradeStudents() const {
        cout << name << " оценивает работы студентов" << endl;
    }
    
    // Метод для расчета премии за стаж (2% от зарплаты за каждый год)
    double calculateBonus() const {
        return salary * experienceYears * 0.02;
    }
};

class Researcher {
protected:
    string researchArea;
    int publicationsCount;

public:
    Researcher(string area, int publications) 
        : researchArea(area), publicationsCount(publications) {}
    
    void conductResearch() const {
        cout << "Проводит исследования в области: " << researchArea << endl;
    }
};

// TODO: Создать класс Professor, наследующий от Teacher и Researcher
class Professor : public Teacher, public Researcher {
public:
    // TODO: Создать конструктор
    
    // Конструктор класса Professor с множественным наследованием от Teacher и Researcher
    Professor(string n, int a, string pos, double sal, string subj, int exp, 
              string area, int publications)
        : Teacher(n, a, pos, sal, subj, exp), Researcher(area, publications) {}
    
    // TODO: Переопределить необходимые методы
    
    // Переопределение метода display() для вывода полной информации о профессоре
    void display() const override {
        cout << "=== Профессор ===" << endl;
        Person::display();  // Вызов метода из класса Person
        cout << "Должность: " << position << endl;
        cout << "Зарплата: " << salary << " руб." << endl;
        cout << "Предмет: " << subject << endl;
        cout << "Стаж: " << experienceYears << " лет" << endl;
        cout << "Область исследований: " << researchArea << endl;
        cout << "Количество публикаций: " << publicationsCount << endl;
    }
    
    // Переопределение метода work() для описания работы профессора
    void work() const override {
        Teacher::work();         // Вызов метода работы учителя
        conductResearch();       // Вызов метода проведения исследований
        cout << "Публикует научные статьи" << endl;
    }
    
    // Метод для руководства диссертациями аспирантов
    void superviseDissertations() const {
        cout << name << " руководит диссертациями аспирантов" << endl;
    }
    
    // Метод для расчета общей зарплаты с надбавками
    double calculateTotalSalary() const {
        double bonus = calculateBonus();                // Премия за стаж
        double publicationBonus = publicationsCount * 500.0;  // Надбавка за публикации (500 руб. за публикацию)
        return salary + bonus + publicationBonus;       // Общая зарплата
    }
};

int main() {
    // TODO: Создать объекты Teacher и Professor
    // Продемонстрировать множественное наследование
    
    cout << "=== Создание объектов ===" << endl << endl;
    
    // Создание объекта Teacher
    Teacher teacher("Иван Петров", 35, "Старший преподаватель", 40000, "Математика", 10);
    teacher.display();
    cout << endl;
    teacher.work();                 // Демонстрация метода work()
    teacher.conductLesson();        // Демонстрация специфичного метода учителя
    teacher.gradeStudents();        // Демонстрация специфичного метода учителя
    cout << "Премия: " << teacher.calculateBonus() << " руб." << endl;
    cout << endl << endl;
    
    // Создание объекта Professor с множественным наследованием
    Professor prof("Анна Смирнова", 45, "Профессор", 80000, "Физика", 20, 
                   "Квантовая механика", 35);
    prof.display();
    cout << endl;
    prof.work();                    // Демонстрация переопределенного метода work()
    prof.superviseDissertations();  // Демонстрация специфичного метода профессора
    cout << "Общая зарплата с надбавками: " << prof.calculateTotalSalary() << " руб." << endl;

}