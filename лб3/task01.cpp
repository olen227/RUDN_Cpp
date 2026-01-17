#include <iostream>  // Ввод-вывод
#include <string>    // Работа со строками
using namespace std;

// Класс для представления студента с инкапсуляцией данных
class Student {
private:
    string name;           // Имя студента
    int age;               // Возраст студента
    double averageGrade;   // Средний балл (вычисляется автоматически)
    int grades[5];         // Массив из 5 оценок

public:
    // Конструктор с инициализацией имени, возраста и обнулением оценок
    Student(string n, int a) : name(n), age(a), averageGrade(0.0), grades{} {}
    
    // Геттеры
    string getName() const{
        return name;
    }
    int getAge() const {
        return age;
    }
    double getAverageGrade() const {
        return averageGrade;
    }
    int getGrade(int index) const { 
        return (index >= 0 && index < 5) ? grades[index] : 0; // Это тернарный оператор, читай его ()?():() как if()then()else()
    }
    
    // Сеттеры с валидацией
    void setName(string n) {
        name = n;
    }
    
    void setAge(int a) {
        if (a > 0 && a < 100) {
            age = a;
        }
    }
    
    void setGrade(int index, int grade) {
        if (index >= 0 && index < 5 && grade >= 2 && grade <= 5) {
            grades[index] = grade;
            calculateAverageGrade();
        }
    }
    
    // Вычисление среднего балла по всем оценкам
    void calculateAverageGrade() {
        int sum = 0;
        for (int grade : grades) {
            sum += grade;
        }
        averageGrade = sum / 5.0;
    }
    
    // Вывод полной информации о студенте
    void displayInfo() const {
        cout << "Студент: " << name << endl;
        cout << "Возраст: " << age << endl;
        cout << "Оценки: ";
        for (int grade : grades) {
            cout << grade << " ";
        }
        cout << "\nСредний балл: " << averageGrade << endl;
    }
    
    // Проверка права на стипендию (средний балл >= 4.5)
    bool hasScholarship() const {
        return averageGrade >= 4.5;
    }
};

int main() {
    // Создание и тестирование объекта Student
    Student student("Иван Иванов", 20);
    
    // Установка оценок
    student.setGrade(0, 5);
    student.setGrade(1, 4);
    student.setGrade(2, 5);
    student.setGrade(3, 3);
    student.setGrade(4, 4);
    
    // Вывод информации и проверка стипендии
    student.displayInfo();

    if (student.hasScholarship()) {
        cout << "Студент получает стипендию" << endl;
    } else {
        cout << "Стипендия не назначена" << endl;
    }

    // Заменил на if-then-else конструкцию для лучшей читаемости
    /*cout << (student.hasScholarship() ?
             "Студент получает стипендию" : 
             "Стипендия не назначена") << endl;*/

}