// B1.3 - Prove runtime binding.
// A pure virtual function cannot simply lose `virtual` (compile error, see
// complete_nonvirtual_total.cpp), so in this experiment Bill::total() is an ordinary
// (non-virtual) function with a body, and the derived classes define their own total().
// Build with -DUSE_VIRTUAL to get the virtual version for comparison.
#include <iostream>
#include <string>

#ifdef USE_VIRTUAL
#define VIRT virtual
#else
#define VIRT
#endif

struct Bill {
    std::string patient;
    explicit Bill(std::string p) : patient(std::move(p)) {}
    virtual ~Bill() = default;
    VIRT double total() const { return 0.0; }   // base version
    virtual std::string type() const { return "Bill"; }
};
struct OutPatientBill : Bill {
    double consultation, tests;
    OutPatientBill(std::string p, double c, double t) : Bill(std::move(p)), consultation(c), tests(t) {}
    double total() const { return consultation + tests; }   // hides (or overrides) Bill::total
    std::string type() const override { return "OP"; }
};
struct InPatientBill : Bill {
    int days;
    double perDay, procedures;
    InPatientBill(std::string p, int d, double r, double s) : Bill(std::move(p)), days(d), perDay(r), procedures(s) {}
    double total() const { return days * perDay + procedures; }
    std::string type() const override { return "IP"; }
};

int main()
{
#ifdef USE_VIRTUAL
    std::cout << "total() IS virtual   (late binding)\n";
#else
    std::cout << "total() NOT virtual  (early binding)\n";
#endif
    OutPatientBill op("Ravi", 500, 1200);
    InPatientBill ip("Meena", 3, 2500, 40000);
    std::cout << "direct object call : OP " << op.total() << ", IP " << ip.total() << '\n';
    Bill* bills[] = {&op, &ip};
    double sum = 0;
    for (Bill* b : bills) {
        std::cout << "through Bill*      : " << b->type() << ' ' << b->patient << " = " << b->total() << '\n';
        sum += b->total();
    }
    std::cout << "day total via Bill*: " << sum << '\n';
}
