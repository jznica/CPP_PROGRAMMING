// B1.4 - Prove the virtual destructor. Build with -DUSE_VIRTUAL_DTOR for the correct version.
#include <iostream>
#include <string>
#include <vector>

#ifdef USE_VIRTUAL_DTOR
#define VDTOR virtual
#else
#define VDTOR
#endif

struct Bill {
    std::string patient;
    explicit Bill(std::string p) : patient(std::move(p)) {}
    VDTOR ~Bill() { std::cout << "~Bill "; }
    virtual double total() const = 0;
};
struct InPatientBill : Bill {
    std::vector<double> dailyCharges;   // owns heap memory
    InPatientBill(std::string p, std::vector<double> d) : Bill(std::move(p)), dailyCharges(std::move(d)) {}
    ~InPatientBill() { std::cout << "~InPatientBill "; }
    double total() const override
    {
        double s = 0;
        for (double c : dailyCharges) s += c;
        return s;
    }
};

int main()
{
    std::cout << std::unitbuf;   // unbuffered: output survives a sanitizer abort
    Bill* b = new InPatientBill("Meena", {2500, 2500, 2500, 40000});
    std::cout << "total " << b->total() << '\n';
    delete b;   // without a virtual destructor only ~Bill runs: undefined behaviour
    std::cout << '\n';
}
