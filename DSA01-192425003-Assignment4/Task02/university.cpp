// Task 2.3 - University people hierarchy with a virtual base class
#include <iostream>
#include <string>

class Fellowship {                                   // has-a (member class)
    std::string agency; double monthly;
public:
    Fellowship(std::string a, double m) : agency(std::move(a)), monthly(m) {}
    std::string str() const { return agency + " Rs." + std::to_string(static_cast<int>(monthly)); }
};

class Person {
protected:
    std::string name; int id; std::string phone;
public:
    Person(std::string n, int i, std::string p) : name(std::move(n)), id(i), phone(std::move(p)) {}
    virtual ~Person() = default;
    virtual void show() const { std::cout << id << ' ' << name << ' ' << phone; }
};

class Student : virtual public Person {              // is-a, public, virtual base
protected:
    std::string programme; double cgpa;
public:
    Student(std::string n, int i, std::string p, std::string prog, double c)
        : Person(std::move(n), i, std::move(p)), programme(std::move(prog)), cgpa(c) {}
    void show() const override { Person::show(); std::cout << " | " << programme << " CGPA " << cgpa; }
};

class Staff : virtual public Person {
protected:
    std::string department; double salary;
public:
    Staff(std::string n, int i, std::string p, std::string d, double s)
        : Person(std::move(n), i, std::move(p)), department(std::move(d)), salary(s) {}
    void show() const override { Person::show(); std::cout << " | " << department << " Rs." << salary; }
};

class TeachingAssistant : public Student, public Staff {   // multiple + hybrid (diamond)
public:
    TeachingAssistant(std::string n, int i, std::string p, std::string prog, double c,
                      std::string d, double s)
        : Person(n, i, p),                                  // most-derived calls virtual base
          Student(n, i, p, std::move(prog), c), Staff(n, i, p, std::move(d), s) {}
    void show() const override
    {
        Person::show();
        std::cout << " | " << programme << " CGPA " << cgpa << " | " << department << " Rs." << salary;
    }
};

class ResearchScholar : public Student {             // multilevel: Person -> Student -> Scholar
    Fellowship fellowship;                           // has-a
public:
    ResearchScholar(std::string n, int i, std::string p, std::string prog, double c, Fellowship f)
        : Person(n, i, p), Student(n, i, p, std::move(prog), c), fellowship(std::move(f)) {}
    void show() const override { Student::show(); std::cout << " | Fellowship " << fellowship.str(); }
};

int main()
{
    Person* people[] = {
        new Student("Kavya", 1, "98400", "B.Tech AIML", 8.6),
        new Staff("Dr. Kumar", 2, "98401", "CSE", 95000),
        new TeachingAssistant("Arun", 3, "98402", "M.Tech", 8.9, "CSE", 20000),
        new ResearchScholar("Divya", 4, "98403", "PhD", 9.1, Fellowship("DST-INSPIRE", 37000))};
    for (Person* p : people) { p->show(); std::cout << '\n'; }
    for (Person* p : people) delete p;
}
