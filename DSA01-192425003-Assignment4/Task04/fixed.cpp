// Task 4.3 - Repaired program: virtual functions, virtual destructor, pointers instead of copies
#include <iostream>
#include <memory>
#include <vector>

class Account {
public:
    virtual double rate() const { return 3.5; }
    virtual ~Account() { std::cout << "  ~Account\n"; }
};

class SeniorAccount : public Account {
    std::unique_ptr<double[]> history = std::make_unique<double[]>(100);   // RAII buffer
public:
    double rate() const override { return 4.0; }
    ~SeniorAccount() override { std::cout << "  ~SeniorAccount (history freed)\n"; }
};

int main()
{
    std::vector<std::unique_ptr<Account>> accounts;          // stores pointers, never copies
    accounts.push_back(std::make_unique<Account>());
    accounts.push_back(std::make_unique<SeniorAccount>());
    std::cout << "rates:\n";
    for (const auto& a : accounts) std::cout << "  " << a->rate() << "\n";

    SeniorAccount s;
    Account& ref = s;                                        // reference: no slicing
    std::cout << "  rate through reference = " << ref.rate() << '\n';
    std::cout << "end of main (automatic cleanup follows):\n";
}
