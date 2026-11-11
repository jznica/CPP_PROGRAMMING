// Q4 - Complete the Code (TODO 1 and TODO 2 completed) + carry test added at the end
// Compile: g++ -std=c++17 -Wall -Wextra complete_the_code.cpp -o ctc && ./ctc
#include <iomanip>
#include <iostream>
class Money {
    long long paise;
public:
    Money(long long rupees = 0, int p = 0) : paise(rupees * 100 + p) {}

    // TODO 1: Overload + as a member function that returns a new Money object
    Money operator+(const Money& other) const {
        Money result;
        result.paise = paise + other.paise;
        return result;
    }

    // TODO 2: Overload == as a member, and << as a friend that prints "Rs 200.00"
    bool operator==(const Money& other) const { return paise == other.paise; }
    friend std::ostream& operator<<(std::ostream& os, const Money& m) {
        long long p = m.paise;
        if (p < 0) { os << '-'; p = -p; }
        char oldFill = os.fill('0');
        os << "Rs " << p / 100 << '.' << std::setw(2) << p % 100;
        os.fill(oldFill);
        return os;
    }
};
int main()
{
    Money a(120, 75), b(79, 25);
    Money c = a + b;
    std::cout << c << '\n' << std::boolalpha << (c == Money(200)) << '\n';

    // Added test: paise carry into rupees (Rs 0.75 + Rs 0.50 = Rs 1.25)
    Money d(0, 75), e(0, 50);
    std::cout << d << " + " << e << " = " << d + e << '\n'
              << ((d + e) == Money(1, 25)) << '\n';
}
