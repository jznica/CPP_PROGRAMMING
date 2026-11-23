// A2 - Predict the Output, variant: `virtual` removed from ~Bill (and `override` from ~OPBill,
// which would otherwise be a compile error because there is nothing to override)
#include <iostream>
struct Bill {
    ~Bill() { std::cout << "~Bill "; }   // CHANGE: virtual removed
    virtual double total() const = 0;
    void show() const { std::cout << total() << ' '; }
};

struct OPBill : Bill {
    ~OPBill() { std::cout << "~OPBill "; }
    double total() const override { return 500; }
};

int main()
{
    Bill* b = new OPBill;
    b->show();
    delete b;
    std::cout << '\n';
}
