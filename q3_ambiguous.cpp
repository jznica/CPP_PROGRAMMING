// Q3 - original courier design (does NOT compile)
#include <iostream>

double charge(double weight)                                      { return weight * 40; }
double charge(double weight, bool express)                        { return weight * (express ? 70 : 40); }
double charge(double weight, bool express = false, bool insured = false)
{
    return weight * (express ? 70 : 40) + (insured ? 25 : 0);
}

int main()
{
    std::cout << charge(2.5) << '\n';        // ambiguous: #1 or #3
    std::cout << charge(2.5, true) << '\n';  // ambiguous: #2 or #3
}
