// Task 3 - Alternative: Student owns a dynamically created Address (Address* home)
#include <chrono>
#include <iostream>
#include <string>
class Address {
    std::string city; int pin;
public:
    Address(const std::string& c, int p) : city(c), pin(p) {}
    std::string str() const { return city + "-" + std::to_string(pin); }
};
class StudentObj { long long regNo; Address home;
public: StudentObj(long long r, const std::string& c, int p) : regNo(r), home(c, p) {} };

class StudentPtr {
    long long regNo; Address* home;
public:
    StudentPtr(long long r, const std::string& c, int p) : regNo(r), home(new Address(c, p)) {}
    StudentPtr(const StudentPtr& o) : regNo(o.regNo), home(new Address(*o.home)) {}   // deep copy
    StudentPtr& operator=(const StudentPtr& o)
    {
        if (this != &o) { Address* fresh = new Address(*o.home); delete home; home = fresh; regNo = o.regNo; }
        return *this;
    }
    ~StudentPtr() { delete home; }                                                    // release
    std::string str() const { return std::to_string(regNo) + " " + home->str(); }
};

int main()
{
    std::cout << "sizeof(StudentObj) = " << sizeof(StudentObj) << " (Address embedded)\n";
    std::cout << "sizeof(StudentPtr) = " << sizeof(StudentPtr) << " + heap block of " << sizeof(Address) << '\n';
    StudentPtr a(192311001, "Salem", 636001), b = a;      // deep copy, no double delete
    std::cout << a.str() << " | " << b.str() << '\n';

    const int N = 1000000; using C = std::chrono::steady_clock;
    auto t0 = C::now(); for (int i = 0; i < N; ++i) { StudentObj s(i, "Salem", 636001); (void)s; }
    auto t1 = C::now(); for (int i = 0; i < N; ++i) { StudentPtr s(i, "Salem", 636001); (void)s; }
    auto t2 = C::now();
    std::cout << "1e6 constructions: member object "
              << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count() << " ms, pointer+new "
              << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << " ms\n";
}
