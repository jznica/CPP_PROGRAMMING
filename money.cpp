// Q4 - Overloading Arithmetic and Stream Operators : Money for a Billing Counter
// Compile: g++ -std=c++17 -Wall -Wextra -O2 money.cpp -o money
// Run    : ./money bill_input.txt
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

class Money {
    long long paise;   // exact integer amount: no rounding errors

public:
    Money(long long rupees = 0, int p = 0) : paise(rupees * 100 + p) {}
    static Money fromPaise(long long p) { Money m; m.paise = p; return m; }
    long long inPaise() const { return paise; }

    // Arithmetic - members: the left operand is always a Money
    Money operator+(const Money& o) const { return fromPaise(paise + o.paise); }   // new object
    Money operator-(const Money& o) const { return fromPaise(paise - o.paise); }
    Money& operator+=(const Money& o) { paise += o.paise; return *this; }           // modifies *this
    Money& operator-=(const Money& o) { paise -= o.paise; return *this; }
    Money operator*(long long qty) const { return fromPaise(paise * qty); }          // price x quantity

    // Comparison - members
    bool operator==(const Money& o) const { return paise == o.paise; }
    bool operator!=(const Money& o) const { return !(*this == o); }
    bool operator<(const Money& o) const { return paise < o.paise; }

    // Stream operators - must be non-members (left operand is a stream); friends for paise
    friend std::ostream& operator<<(std::ostream& os, const Money& m);
    friend std::istream& operator>>(std::istream& is, Money& m);
};

std::ostream& operator<<(std::ostream& os, const Money& m) {
    long long p = m.paise;
    if (p < 0) { os << '-'; p = -p; }
    char oldFill = os.fill('0');
    os << "Rs " << p / 100 << '.' << std::setw(2) << p % 100;
    os.fill(oldFill);
    return os;
}

// Reads amounts such as  120.75   Rs120.75   -15.5   200
std::istream& operator>>(std::istream& is, Money& m) {
    std::string tok;
    if (!(is >> tok)) return is;
    if (tok.rfind("Rs", 0) == 0) tok = tok.substr(2);
    bool neg = !tok.empty() && tok[0] == '-';
    if (neg) tok = tok.substr(1);
    std::size_t dot = tok.find('.');
    std::string rs = tok.substr(0, dot), ps = dot == std::string::npos ? "" : tok.substr(dot + 1);
    if (rs.empty() || rs.find_first_not_of("0123456789") != std::string::npos ||
        ps.size() > 2 || ps.find_first_not_of("0123456789") != std::string::npos) {
        is.setstate(std::ios::failbit);
        return is;
    }
    if (ps.size() == 1) ps += '0';
    long long total = std::stoll(rs) * 100 + (ps.empty() ? 0 : std::stoll(ps));
    m = Money::fromPaise(neg ? -total : total);
    return is;
}

int main(int argc, char** argv) {
    std::cout << "--- 1. Carry of paise into rupees ---\n";
    Money x(0, 75), y(0, 50);
    std::cout << x << " + " << y << " = " << x + y << '\n';

    std::cout << "\n--- 2. Subtraction producing a negative amount ---\n";
    Money paid(100), due(115, 50);
    Money change = paid - due;
    std::cout << paid << " - " << due << " = " << change << '\n';
    if (change < Money(0))
        std::cout << "Decision: negative result kept (it means the customer still owes "
                  << Money(0) - change << "), shown with a '-' sign, never silently clamped.\n";

    std::cout << "\n--- 3. Bill calculated with overloaded operators ---\n";
    const char* file = argc > 1 ? argv[1] : "bill_input.txt";
    std::ifstream in(file);
    if (!in) { std::cerr << "cannot open " << file << '\n'; return 1; }
    std::string item;
    Money price, subtotal;
    long long qty;
    while (in >> item >> price >> qty) {
        Money line = price * qty;
        subtotal += line;
        std::cout << "  " << std::left << std::setw(12) << item << std::right << price << " x "
                  << qty << " = " << line << '\n';
    }
    Money discount(50);
    Money total = subtotal < discount ? Money(0) : subtotal - discount;
    std::cout << "  Subtotal = " << subtotal << "\n  Discount = " << discount
              << "\n  Total    = " << total << '\n';

    std::cout << "\n--- 4. Reading amounts with >> ---\n";
    std::istringstream ss("Rs12.5 99.99 -3.05 abc");
    Money m;
    while (ss >> m) std::cout << "  read " << m << '\n';
    std::cout << "  'abc' rejected (failbit set)\n";

    std::cout << "\n--- 5. long long paise vs double rupees : 1,000,000 additions of Rs 0.10 ---\n";
    const int N = 1000000;
    Money exact;
    volatile long long tenPaise = 10;   // volatile: stop the compiler folding the loop
    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < N; ++i) exact += Money::fromPaise(tenPaise);
    auto t1 = std::chrono::steady_clock::now();
    volatile double step = 0.10;
    double approx = 0.0;
    auto t2 = std::chrono::steady_clock::now();
    for (int i = 0; i < N; ++i) approx += step;
    auto t3 = std::chrono::steady_clock::now();
    double msMoney = std::chrono::duration<double, std::milli>(t1 - t0).count();
    double msDouble = std::chrono::duration<double, std::milli>(t3 - t2).count();
    std::cout << "  Money  result : " << exact << "          time " << std::fixed << std::setprecision(3) << msMoney << " ms\n";
    std::cout << "  double result : " << std::setprecision(11) << approx << "   time " << std::setprecision(3) << msDouble << " ms\n";
    std::cout << std::scientific << std::setprecision(3) << "  double error  : " << approx - 100000.0 <<
     " rupees\n";
    std::cout << "  double == 100000.00 ? " << std::boolalpha << (approx == 100000.0) << '\n';
}
