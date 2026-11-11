// Q7 - Redesigned SavingsAccount: circular buffer with a start index instead of shifting the array.
// Same interface and same test as Q4/q4_account.cpp; the output must be identical.
#include <iostream>
#include <iomanip>
#include <string>

struct Transaction {
    char type;        // 'D' deposit, 'W' withdrawal
    double amount;
    double balanceAfter;
};

class SavingsAccount {
    static const int MAX_TXN = 10;     // change to 50 -> only this line changes
    std::string accNo;
    double balance;
    Transaction txn[MAX_TXN];       // circular buffer: logical i-th oldest is txn[(start + i) % MAX_TXN]
    int start;                      // NEW: index of the oldest record
    int count;                      // how many are stored (<= MAX_TXN)

    void record(char type, double amt);   // private helper, defined outside

public:
    // defined INSIDE the class (implicitly inline)
    SavingsAccount(const std::string& no, double opening)
        : accNo(no), balance(opening), start(0), count(0) {}
    double getBalance() const { return balance; }

    // declared here, defined OUTSIDE with ::
    void deposit(double amt);
    bool withdraw(double amt);
    void miniStatement() const;
};

void SavingsAccount::record(char type, double amt)
{
    int pos = (start + count) % MAX_TXN;       // one write, O(1), whatever MAX_TXN is
    txn[pos] = {type, amt, balance};
    if (count < MAX_TXN) ++count;
    else start = (start + 1) % MAX_TXN;        // buffer full: the oldest was just overwritten
}

void SavingsAccount::deposit(double amt)
{
    if (amt <= 0) return;
    balance += amt;
    record('D', amt);
}

bool SavingsAccount::withdraw(double amt)
{
    if (amt <= 0 || amt > balance) return false;   // invariant: balance >= 0
    balance -= amt;
    record('W', amt);
    return true;
}

void SavingsAccount::miniStatement() const
{
    std::cout << "Mini-statement  A/c " << accNo << "  (last " << count << ")\n";
    for (int i = 0; i < count; ++i) {
        const Transaction& t = txn[(start + i) % MAX_TXN];                       // oldest -> newest
        std::cout << std::setw(3) << i + 1 << ". " << (t.type == 'D' ? "DEP " : "WDL ")
                  << std::setw(8) << t.amount << "   bal " << std::setw(8) << t.balanceAfter << '\n';
    }
    std::cout << "Current balance: " << balance << '\n';
}

int main()
{
    std::cout << std::fixed << std::setprecision(0);
    SavingsAccount acc("SB-1029", 5000);
    for (int i = 1; i <= 12; ++i) {          // 12 transactions -> first 2 must disappear
        if (i % 3 == 0) acc.withdraw(100.0 * i);
        else            acc.deposit(100.0 * i);
    }
    std::cout << "withdraw 999999 accepted? " << std::boolalpha << acc.withdraw(999999) << '\n';
    acc.miniStatement();
}
