// Task 2 - Alternative: TA HAS a Student and a Staff (composition) vs virtual inheritance
#include <iostream>
#include <string>
struct Person { std::string name; int id; std::string phone; };
struct VStudent : virtual Person { std::string programme; double cgpa; };
struct VStaff   : virtual Person { std::string department; double salary; };
struct VTA      : VStudent, VStaff {};

struct CStudent : Person { std::string programme; double cgpa; };
struct CStaff   : Person { std::string department; double salary; };
struct CTA { CStudent asStudent; CStaff asStaff; };   // two Person sub-objects

int main()
{
    std::cout << "sizeof(Person)             = " << sizeof(Person) << '\n';
    std::cout << "sizeof(VTA)  virtual bases = " << sizeof(VTA) << "  (one Person)\n";
    std::cout << "sizeof(CTA)  composition   = " << sizeof(CTA) << "  (two Persons)\n";
    VTA v; v.name = "Arun";                 // unique, unambiguous
    CTA c; c.asStudent.name = "Arun"; c.asStaff.name = "Arun K";   // must choose; can diverge
    std::cout << v.name << " | " << c.asStudent.name << " vs " << c.asStaff.name << '\n';
}
