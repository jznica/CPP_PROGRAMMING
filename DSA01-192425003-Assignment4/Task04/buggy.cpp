// Task 4.2 - The original (defective) bank program with destructor output added as evidence
#include <iostream>
#include <vector>

class Account {
public:
    double rate() const { return 3.5; }                     // DEFECT 1: not virtual
    ~Account() { std::cout << "  ~Account\n"; }             // DEFECT 2: not virtual
};

class SeniorAccount : public Account {
    double* history = new double[100];
public:
    double rate() const { return 4.0; }
    ~SeniorAccount() { std::cout << "  ~SeniorAccount (history freed)\n"; delete[] history; }
};

int main()
{
    Account* accounts[2] = { new Account, new SeniorAccount };
    std::cout << "rates:\n";
    for (Account* a : accounts) std::cout << "  " << a->rate() << "\n";
    std::cout << "deleting through Account*:\n";
    for (Account* a : accounts) delete a;
    std::cout << "slicing copy:\n";
    std::vector<Account> v;
    v.push_back(SeniorAccount());                           // DEFECT 3: object slicing
    std::cout << "  rate of copy in vector = " << v[0].rate() << '\n';
    std::cout << "end of main\n";
}
