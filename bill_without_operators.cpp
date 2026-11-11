// Q4 readability comparison: the same bill written with named functions (no operator overloading)
// Compile: g++ -std=c++17 -Wall -Wextra bill_without_operators.cpp -o bill_fn && ./bill_fn
#include <cstdio>

struct Money { long long paise; };
Money make(long long rupees, int p) { return Money{rupees * 100 + p}; }
Money add(Money a, Money b) { return Money{a.paise + b.paise}; }
Money sub(Money a, Money b) { return Money{a.paise - b.paise}; }
Money mul(Money a, long long q) { return Money{a.paise * q}; }
bool less(Money a, Money b) { return a.paise < b.paise; }
void print(Money m) { std::printf("Rs %lld.%02lld", m.paise / 100, m.paise % 100); }

int main() {
    Money subtotal = make(0, 0);
    subtotal = add(subtotal, mul(make(45, 50), 4));
    subtotal = add(subtotal, mul(make(12, 75), 10));
    subtotal = add(subtotal, mul(make(649, 0), 1));
    subtotal = add(subtotal, mul(make(89, 25), 2));
    Money discount = make(50, 0);
    Money total = less(subtotal, discount) ? make(0, 0) : sub(subtotal, discount);
    std::printf("Total = "); print(total); std::printf("\n");
    // Operator version of the same lines:
    //   subtotal += Money(45, 50) * 4;  ...
    //   Money total = subtotal < discount ? Money(0) : subtotal - discount;
    //   std::cout << "Total = " << total << '\n';
}
