#include <iostream>
double charge(double kg, bool express = false, bool insured = false) { return kg * (express ? 70 : 40) + (insured ? 25 : 0); }
double charge(int grams, bool express = false, bool insured = false) { return grams / 1000.0 * (express ? 70 : 40) + (insured ? 25 : 0); }
int main() { std::cout << charge(2L) << '\n'; }   // long -> int and long -> double are both conversions: ambiguous
