// Q3 - redesigned courier charge calculator
#include <iostream>
#include <iomanip>

const double RATE_PER_KG = 40.0, EXPRESS_PER_KG = 70.0, INSURANCE = 25.0;

struct Parcel {
    double kg;
    bool express;
    bool insured;
};

// (A) weight in kg - the one function that does the arithmetic
double charge(double kg, bool express = false, bool insured = false)
{
    std::cout << "[charge(double kg, ...)]      ";
    return kg * (express ? EXPRESS_PER_KG : RATE_PER_KG) + (insured ? INSURANCE : 0);
}

// (B) weight in grams - different TYPE, so no clash with (A)
double charge(int grams, bool express = false, bool insured = false)
{
    std::cout << "[charge(int grams, ...)]      ";
    return (grams / 1000.0) * (express ? EXPRESS_PER_KG : RATE_PER_KG) + (insured ? INSURANCE : 0);
}

// (C) a whole Parcel - no defaults needed, the object carries the options
double charge(const Parcel& p)
{
    std::cout << "[charge(const Parcel&)]       ";
    return p.kg * (p.express ? EXPRESS_PER_KG : RATE_PER_KG) + (p.insured ? INSURANCE : 0);
}

int main()
{
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "1 charge(2)              -> " << charge(2) << '\n';
    std::cout << "2 charge(2.0f)           -> " << charge(2.0f) << '\n';
    std::cout << "3 charge(2.5)            -> " << charge(2.5) << '\n';
    std::cout << "4 charge(1500, true)     -> " << charge(1500, true) << '\n';
    std::cout << "5 charge(1.2, true, true)-> " << charge(1.2, true, true) << '\n';
    std::cout << "6 charge(Parcel{3,0,1})  -> " << charge(Parcel{3.0, false, true}) << '\n';
    std::cout << "7 charge('A')            -> " << charge('A') << '\n';
    // charge(2L);   // error: long -> int and long -> double are both conversions: ambiguous
}
