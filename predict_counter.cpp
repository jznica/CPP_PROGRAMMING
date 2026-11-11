// Q5 - Predict the Output (program copied exactly from the assignment)
// Compile: g++ -std=c++17 -Wall -Wextra predict_counter.cpp -o predict_counter && ./predict_counter
#include <iostream>
struct Counter {
 int v;
 Counter& operator++() { ++v; return *this; } // prefix
 Counter operator++(int) { Counter old = *this; ++v; return old; } // postfix
};
int main()
{
 Counter c{5};
 Counter a = c++;
 Counter b = ++c;
 std::cout << a.v << ' ' << b.v << ' ' << c.v << '\n';
}
