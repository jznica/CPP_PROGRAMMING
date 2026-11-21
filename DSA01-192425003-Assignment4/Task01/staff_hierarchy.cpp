// Task 1.2 - Repaired payroll design: common base class + overridden pay()
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class Staff {                                   // abstract base: common data written once
    std::string name;
    int id;

public:
    Staff(std::string n, int i) : name(std::move(n)), id(i) {}
    virtual ~Staff() = default;
    virtual double pay() const = 0;             // every category must define its own rule
    virtual std::string category() const = 0;
    const std::string& getName() const { return name; }
    int getId() const { return id; }
};

class SalariedStaff : public Staff {            // basic + DA rule written once
protected:
    static constexpr double DA_RATE = 0.42;     // named constant instead of magic number
    double basic;

public:
    SalariedStaff(std::string n, int i, double b) : Staff(std::move(n), i), basic(b) {}
    double da() const { return basic * DA_RATE; }
    double pay() const override { return basic + da(); }
};

class Professor : public SalariedStaff {
    double research;

public:
    Professor(std::string n, int i, double b, double r)
        : SalariedStaff(std::move(n), i, b), research(r) {}
    double pay() const override { return SalariedStaff::pay() + research; }
    std::string category() const override { return "Professor"; }
};

class LabAssistant : public SalariedStaff {
public:
    using SalariedStaff::SalariedStaff;        // inherits the constructor
    std::string category() const override { return "LabAssistant"; }
};

class GuestFaculty : public Staff {
    double hours, rate;

public:
    GuestFaculty(std::string n, int i, double h, double r)
        : Staff(std::move(n), i), hours(h), rate(r) {}
    double pay() const override { return hours * rate; }
    std::string category() const override { return "GuestFaculty"; }
};

// New category added later WITHOUT touching any class above
class Visiting : public Staff {
    double perLecture; int lectures;

public:
    Visiting(std::string n, int i, double p, int l) : Staff(std::move(n), i), perLecture(p), lectures(l) {}
    double pay() const override { return perLecture * lectures; }
    std::string category() const override { return "Visiting"; }
};

int main()
{
    std::vector<std::unique_ptr<Staff>> staff;
    staff.push_back(std::make_unique<Professor>("Anitha", 101, 80000, 15000));
    staff.push_back(std::make_unique<LabAssistant>("Ravi", 202, 30000));
    staff.push_back(std::make_unique<GuestFaculty>("Meena", 303, 40, 1200));
    staff.push_back(std::make_unique<Visiting>("Dr. Rao", 404, 5000, 6));

    double total = 0;
    std::cout << std::fixed << std::setprecision(2);
    for (const auto& s : staff) {               // ONE loop processes every category
        std::cout << std::left << std::setw(13) << s->category() << std::setw(8) << s->getName()
                  << " id=" << s->getId() << "  pay=" << s->pay() << '\n';
        total += s->pay();
    }
    std::cout << "Total payroll = " << total << '\n';
}
