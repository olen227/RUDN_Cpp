#include <iostream>              // ввод-вывод
#include <string>                // строки
#include <vector>                // динамические массивы
#include <memory>                // unique_ptr
#include <algorithm>             // find
using namespace std;             // std::

/* Абстрактный базовый класс */
class UniversityMember {
protected:
    string name;                 // имя
    string id;                   // идентификатор
    string email;                // почта
public:
    UniversityMember(string n, string i, string e) : name(n), id(i), email(e) {}
    virtual ~UniversityMember() {}

    // чисто виртуальные
    virtual void displayInfo() const = 0;
    virtual void work() const = 0;
    virtual string getRole() const = 0;

    // публичные геттеры (чтобы не ломать инкапсуляцию и не лезть к protected извне)
    const string& getName() const {
        return name;
    }

    const string& getId() const {
        return id;
    }

    const string& getEmail() const {
        return email;
    }
};

/* Студент */
class Student : public UniversityMember {
private:
    string major;               // направление подготовки
    int year;                   // курс
    double gpa;                 // средний балл
    vector<string> courses;     // список курсов
public:
    Student(string n, string i, string e, string maj, int y, double g = 0.0)
        : UniversityMember(n, i, e), major(maj), year(y), gpa(g) {}

    void displayInfo() const override {
        cout << "[Студент] " << name << " (" << id << "), email: " << email
             << ", спец.: " << major << ", курс: " << year
             << ", GPA: " << gpa << "\n";
        cout << "Курсы: ";
        for (auto& c : courses) {
            cout << c << " ";
        }
        cout << "\n";
    }

    void work() const override {
        cout << name << " учится по программе \"" << major << "\"\n";
    }

    string getRole() const override {
        return "Студент";
    }

    void enrollCourse(string& course) {
        if (find(courses.begin(), courses.end(), course) == courses.end()) {
            courses.push_back(course);
        }
    }

    void dropCourse(string& course) {
        auto it = find(courses.begin(), courses.end(), course);
        if (it != courses.end()) {
            courses.erase(it);
        }
    }

    double calculateGPA() const {
        return gpa;
    }

    void setGPA(double v) {
        if (v >= 0.0 && v <= 5.0) {
            gpa = v;
        }
    }
};

/* Профессор */
class Professor : public UniversityMember {
private:
    string department;              // кафедра
    string office;                  // кабинет
    double salary;                  // базовая зарплата
    vector<string> teachingCourses; // ведёт курсы
public:
    Professor(string n, string i, string e, string dept, string off, double sal)
        : UniversityMember(n, i, e), department(dept), office(off), salary(sal) {}

    void displayInfo() const override {
        cout << "[Профессор] " << name << " (" << id << "), email: " << email
             << ", каф.: " << department << ", каб.: " << office
             << ", оклад: " << salary << "\n";
        cout << "Ведёт курсы: ";
        for (auto& c : teachingCourses) {
            cout << c << " ";
        }
        cout << "\n";
    }

    void work() const override {
        cout << name << " читает лекции на кафедре \"" << department << "\"\n";
    }

    string getRole() const override {
        return "Профессор";
    }

    void assignCourse(const string& course) {
        if (find(teachingCourses.begin(), teachingCourses.end(), course) == teachingCourses.end()) {
            teachingCourses.push_back(course);
        }
    }

    void removeCourse(const string& course) {
        auto it = find(teachingCourses.begin(), teachingCourses.end(), course);
        if (it != teachingCourses.end()) {
            teachingCourses.erase(it);
        }
    }

    double calculateSalary() const {
        return salary + teachingCourses.size() * 5000.0;
    }
};

/* Курс (композиция: хранит указатели, не владеет) */
class Course {
private:
    string courseCode;                  // код курса
    string courseName;                  // название
    int credits;                        // кредиты
    Professor* instructor;              // преподаватель (невладеющий указатель)
    vector<Student*> enrolledStudents;  // студенты (невладеющие указатели)
public:
    Course(string code, string name, int cred)
        : courseCode(code), courseName(name), credits(cred), instructor(nullptr) {}

    void addStudent(Student* student) {
        if (!student) {
            return;
        }

        if (find(enrolledStudents.begin(), enrolledStudents.end(), student) == enrolledStudents.end()) {
            enrolledStudents.push_back(student);
            student->enrollCourse(courseCode);
        }
    }

    void removeStudent(const string& studentId) {
        for (auto it = enrolledStudents.begin(); it != enrolledStudents.end(); ++it) {
            if ((*it)->getId() == studentId) {
                (*it)->dropCourse(courseCode);
                enrolledStudents.erase(it);
                return;
            }
        }
    }

    void setInstructor(Professor* prof) {
        instructor = prof;
        if (instructor) {
            instructor->assignCourse(courseCode);
        }
    }

    void displayCourseInfo() const {
        cout << "[Курс] " << courseCode << " — " << courseName
             << ", кредиты: " << credits << "\n";
        cout << "Преподаватель: " << (instructor ? instructor->getName() : string("не назначен")) << "\n";
        cout << "Записано студентов: " << enrolledStudents.size() << "\n";
    }

    const string& getCode() const {
        return courseCode;
    }
};

/* Университет (владеет объектами через unique_ptr) */
class University {
private:
    string name;
    vector<unique_ptr<UniversityMember>> members;
    vector<unique_ptr<Course>> courses;
    static int totalUniversities;

public:
    University(string n) : name(n) {
        totalUniversities++;
    }

    ~University() {
        totalUniversities--;
    }

    void addStudent(unique_ptr<Student> s) {
        members.push_back(move(s));
    }

    void addProfessor(unique_ptr<Professor> p) {
        members.push_back(move(p));
    }

    void addCourse(unique_ptr<Course> c) {
        courses.push_back(move(c));
    }

    Student* findStudentById(const string& sid) {
        for (auto& m : members) {
            if (m->getRole() == "Студент" && m->getId() == sid) {
                return static_cast<Student *>(m.get());
            }
        }
        return nullptr;
    }

    Professor* findProfessorById(const string& pid) {
        for (auto& m : members) {
            if (m->getRole() == "Профессор" && m->getId() == pid) {
                return static_cast<Professor *>(m.get());
            }
        }
        return nullptr;
    }

    Course* findCourseByCode(const string& code) {
        for (auto& c : courses) {
            if (c->getCode() == code) {
                return c.get();
            }
        }
        return nullptr;
    }

    void showAll() const {
        cout << "=== Члены университета \"" << name << "\" ===\n";
        for (auto& m : members) {
            m->displayInfo();
        }
        cout << "=== Курсы ===\n";
        for (auto& c : courses) {
            c->displayCourseInfo();
        }
    }

    static int getTotalUniversities() {
        return totalUniversities;
    }
};
int University::totalUniversities = 0;

/* Демонстрация */
int main() {
    University u("bmstu");  // как просил

    auto s1 = make_unique<Student>("Иван Петров",   "S001", "s1@bmstu.ru", "CS",   2, 4.3);
    auto s2 = make_unique<Student>("Мария Смирнова","S002", "s2@bmstu.ru", "Math", 1, 4.7);
    auto p1 = make_unique<Professor>("А.А. Проф",   "P001", "p1@bmstu.ru", "КН", "A-101", 80000);
    auto c1 = make_unique<Course>("CS101", "Введение в программирование", 4);

    u.addStudent(move(s1));
    u.addStudent(move(s2));
    u.addProfessor(move(p1));
    u.addCourse(move(c1));

    auto* prof   = u.findProfessorById("P001");
    auto* stud   = u.findStudentById("S001");
    auto* course = u.findCourseByCode("CS101");

    if (course && prof) course->setInstructor(prof);
    if (course && stud) course->addStudent(stud);

    u.showAll();
}