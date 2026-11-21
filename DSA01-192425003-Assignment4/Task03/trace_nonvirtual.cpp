// Task 3.4 - Experiment: trace.cpp with the virtual keyword removed from both base classes
#include <iostream>
#include <string>

class Address {                                         // member class of Person
    std::string city; int pin;
public:
    Address(std::string c, int p) : city(std::move(c)), pin(p) { std::cout << "  Address()\n"; }
    ~Address() { std::cout << "  ~Address()\n"; }
    std::string str() const { return city + "-" + std::to_string(pin); }
};

class Course {                                          // member class of Student
    std::string code;
public:
    explicit Course(std::string c) : code(std::move(c)) { std::cout << "  Course()\n"; }
    ~Course() { std::cout << "  ~Course()\n"; }
    const std::string& str() const { return code; }
};

class Person {
protected:
    std::string name; Address addr;
public:
    Person(std::string n, std::string city, int pin)
        : name(std::move(n)), addr(std::move(city), pin) { std::cout << "  Person()\n"; }
    virtual ~Person() { std::cout << "  ~Person()\n"; }
};

class Student : public Person {
protected:
    double cgpa; Course course;
public:
    Student(std::string n, std::string city, int pin, double c, std::string code)
        : Person(std::move(n), std::move(city), pin), cgpa(c), course(std::move(code))
    { std::cout << "  Student()\n"; }
    ~Student() override { std::cout << "  ~Student()\n"; }
};

class Staff : public Person {
protected:
    double salary;
public:
    Staff(std::string n, std::string city, int pin, double s)
        : Person(std::move(n), std::move(city), pin), salary(s) { std::cout << "  Staff()\n"; }
    ~Staff() override { std::cout << "  ~Staff()\n"; }
};

class TeachingAssistant : public Student, public Staff {
public:
    TeachingAssistant(const std::string& n, const std::string& city, int pin,
                      double c, const std::string& code, double s)
        : Student(n, city, pin, c, code),        // Person(...) here is now an error: not a direct base
          Staff(n, city, pin, s)                 // each base builds its OWN Person
    { std::cout << "  TeachingAssistant()\n"; }
    ~TeachingAssistant() override { std::cout << "  ~TeachingAssistant()\n"; }
    void show() const { std::cout << "  [" << Student::name << ", " << Student::addr.str() << ", " << course.str()
                                  << ", CGPA " << cgpa << ", Rs." << salary << "]\n"; }
};

int main()
{
    std::cout << "--- Student ---\n";
    { Student s("Kavya", "Salem", 636001, 8.6, "DSA01"); }
    std::cout << "--- Staff ---\n";
    { Staff st("Dr. Kumar", "Chennai", 600077, 95000); }
    std::cout << "--- TeachingAssistant ---\n";
    { TeachingAssistant ta("Arun", "Chennai", 602105, 8.9, "CSA14", 20000); ta.show(); }
    std::cout << "sizeof(TeachingAssistant) = " << sizeof(TeachingAssistant) << '\n';
}
