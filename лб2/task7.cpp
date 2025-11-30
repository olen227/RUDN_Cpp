#include <iostream>
#include <string>
#include <iomanip>
using namespace std;

struct Date {
    int day;
    int month;
    int year;
};

struct Student {
    string fullName;
    Date birthDate;
    int grades[5];
};

void printStudent(const Student& s) {
    cout << "\n--- Информация о студенте ---\n";
    cout << "ФИО: " << s.fullName << "\n";
    cout << "Дата рождения: " 
         << setfill('0') << setw(2) << s.birthDate.day << "."
         << setfill('0') << setw(2) << s.birthDate.month << "."
         << s.birthDate.year << "\n";
    
    cout << "Оценки за сессию: ";
    for (int i = 0; i < 5; i++) {
        cout << s.grades[i];
        if (i < 4) cout << ", ";
    }
    cout << "\n";
}

double getAverageRating(const Student& s) {
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += s.grades[i];
    }
    return static_cast<double>(sum) / 5.0;
}

bool isValidGrade(int grade) {
    return grade >= 2 && grade <= 5;
}

bool isValidDate(const Date& date) {
    if (date.year < 1900 || date.year > 2010) return false;
    if (date.month < 1 || date.month > 12) return false;
    if (date.day < 1 || date.day > 31) return false;
    return true;
}

int main() {
    cout << "=== УЧЕТ СТУДЕНТОВ В ГРУППЕ ===\n";
    
    Student group[] = {
        {"Иванов Иван Иванович", {15, 3, 2003}, {5, 5, 4, 5, 5}},
        {"Петрова Мария Сергеевна", {22, 7, 2002}, {4, 4, 5, 4, 4}},
        {"Сидоров Алексей Петрович", {10, 12, 2003}, {3, 3, 4, 3, 4}},
        {"Козлова Анна Дмитриевна", {5, 9, 2002}, {5, 5, 5, 5, 5}},
        {"Смирнов Дмитрий Андреевич", {18, 1, 2003}, {4, 5, 4, 5, 4}}
    };
    
    const int GROUP_SIZE = sizeof(group) / sizeof(Student);
    
    cout << "\nПроверка корректности данных...\n";
    bool allValid = true;
    
    for (int i = 0; i < GROUP_SIZE; i++) {
        if (!isValidDate(group[i].birthDate)) {
            cerr << "Некорректная дата у студента: " << group[i].fullName << "\n";
            allValid = false;
        }
        
        for (int j = 0; j < 5; j++) {
            if (!isValidGrade(group[i].grades[j])) {
                cerr << "Некорректная оценка у студента: " << group[i].fullName << "\n";
                allValid = false;
                break;
            }
        }
    }
    
    if (!allValid) {
        cerr << "\nОбнаружены ошибки в данных. Исправьте и запустите снова.\n";
        return 1;
    }
    
    cout << "Все данные корректны.\n";
    
    cout << "\n=== СПИСОК ВСЕХ СТУДЕНТОВ ===";
    for (int i = 0; i < GROUP_SIZE; i++) {
        printStudent(group[i]);
        double avg = getAverageRating(group[i]);
        cout << "Средний балл: " << fixed << setprecision(2) << avg << "\n";
    }
    
    cout << "\n=== СТУДЕНТЫ СО СРЕДНИМ БАЛЛОМ ВЫШЕ 4.0 ===\n";
    bool foundAny = false;
    
    for (int i = 0; i < GROUP_SIZE; i++) {
        double avg = getAverageRating(group[i]);
        if (avg > 4.0) {
            cout << group[i].fullName 
                 << " (средний балл: " << fixed << setprecision(2) << avg << ")\n";
            foundAny = true;
        }
    }
    
    if (!foundAny) {
        cout << "Нет студентов с баллом выше 4.0\n";
    }
    
    cout << "\n=== СТАТИСТИКА ГРУППЫ ===\n";
    double totalAvg = 0;
    for (int i = 0; i < GROUP_SIZE; i++) {
        totalAvg += getAverageRating(group[i]);
    }
    totalAvg /= GROUP_SIZE;
    
    cout << "Всего студентов: " << GROUP_SIZE << "\n";
    cout << "Средний балл группы: " << fixed << setprecision(2) << totalAvg << "\n";
    
    return 0;
}
