// Q4 alternative: member operator vs non-member friend vs named functions.
// g++ -std=c++17 -Wall -Wextra member_vs_friend.cpp            -> compiles (friend version)
// g++ -std=c++17 -Wall -Wextra -DMEMBER member_vs_friend.cpp  -> 100 + m fails to compile
#include <iomanip>
#include <iostream>

class Money {
    long long paise;
public:
    Money(long long rupees = 0, int p = 0) : paise(rupees * 100 + p) {}   // non-explicit: int -> Money
#ifdef MEMBER
    Money operator+(const Money& o) const { Money r; r.paise = paise + o.paise; return r; }
#else
    friend Money operator+(const Money& a, const Money& b) { Money r; r.paise = a.paise + b.paise; return r; }
#endif
    // Named-function alternative (no operator overloading at all)
    friend Money add(const Money& a, const Money& b) { Money r; r.paise = a.paise + b.paise; return r; }
    friend bool equals(const Money& a, const Money& b) { return a.paise == b.paise; }
    friend std::ostream& operator<<(std::ostream& os, const Money& m) {
        char f = os.fill('0');
        os << "Rs " << m.paise / 100 << '.' << std::setw(2) << m.paise % 100;
        os.fill(f);
        return os;
    }
};

int main() {
    Money m(20, 50);
    std::cout << "m + 100 = " << m + 100 << '\n';   // fine in both: 100 converts to Money (right operand)
    std::cout << "100 + m = " << 100 + m << '\n';   // member: no int::operator+(Money) -> ERROR
    std::cout << "add(100, m) = " << add(100, m) << ", equals = " << std::boolalpha
              << equals(add(100, m), Money(120, 50)) << '\n';
    std::cout << "sizeof(Money) = " << sizeof(Money) << " bytes in every version\n";
}
