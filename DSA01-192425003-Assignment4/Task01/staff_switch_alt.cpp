// Task 1 - Alternative: one Staff class with a type field and a switch in pay()
#include <iostream>
#include <string>

enum class Type { Professor, LabAssistant, GuestFaculty };

struct StaffSwitch {
    std::string name; int id; Type type;
    double basic = 0, research = 0, hours = 0, rate = 0;   // every object carries every field
    double pay() const
    {
        switch (type) {
        case Type::Professor:    return basic + basic * 0.42 + research;
        case Type::LabAssistant: return basic + basic * 0.42;
        case Type::GuestFaculty: return hours * rate;
        }
        return 0;
    }
};

struct StaffBase { std::string name; int id; virtual ~StaffBase() = default; virtual double pay() const = 0; };
struct Prof : StaffBase { double basic, research; double pay() const override { return basic * 1.42 + research; } };
struct Lab  : StaffBase { double basic;           double pay() const override { return basic * 1.42; } };
struct Guest: StaffBase { double hours, rate;     double pay() const override { return hours * rate; } };

int main()
{
    StaffSwitch p{"Anitha", 101, Type::Professor, 80000, 15000};
    StaffSwitch g{"Meena", 303, Type::GuestFaculty, 0, 0, 40, 1200};
    std::cout << "switch pay: " << p.pay() << ' ' << g.pay() << '\n';
    std::cout << "sizeof(StaffSwitch)   = " << sizeof(StaffSwitch) << " bytes (all categories)\n";
    std::cout << "sizeof(Prof)          = " << sizeof(Prof) << " bytes\n";
    std::cout << "sizeof(Lab)           = " << sizeof(Lab) << " bytes\n";
    std::cout << "sizeof(Guest)         = " << sizeof(Guest) << " bytes\n";
}
