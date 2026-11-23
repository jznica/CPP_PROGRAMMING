// A2.2 evidence - let g++ show the real vtables of Bill, OutPatientBill, InPatientBill
// and the machine code of a call b->total() through a Bill pointer.
//   g++ -std=c++17 -fdump-lang-class -c vtable_dump.cpp      (writes *.class file)
//   g++ -std=c++17 -O2 -S vtable_dump.cpp -o -               (assembly of callTotal)
#include "../TaskB1/billing.hpp"

double callTotal(const hb::Bill* b) { return b->total(); }

int main()
{
    hb::OutPatientBill o(1, "Ravi", 500, 1200);
    hb::InPatientBill i(2, "Meena", 3, 2500, 40000, 4500);
    return static_cast<int>(callTotal(&o) + callTotal(&i)) > 0 ? 0 : 1;
}
