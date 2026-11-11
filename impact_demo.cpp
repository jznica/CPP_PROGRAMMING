// Task 12 - worked example 1: add Airport to the enum and change nothing else
// Compile: g++ -std=c++17 -Wall -Wextra impact_demo.cpp -o impact_demo
#include <iostream>
enum class Zone { Inner, Middle, Outer, Airport };   // Airport added ...
constexpr long long BASE_PAISE[] = {1000, 2000, 3000};

long long fare(Zone z, bool peak)                     // ... but the v1 switch is unchanged
{
    long long base = 0;
    switch (z) {
    case Zone::Inner:  base = BASE_PAISE[0]; break;
    case Zone::Middle: base = BASE_PAISE[1]; break;
    case Zone::Outer:  base = BASE_PAISE[2]; break;
    }
    return peak ? base * 120 / 100 : base;
}

int main()
{
    std::cout << "Airport off-peak fare = " << fare(Zone::Airport, false) << " paise\n";
    std::cout << "Airport peak fare     = " << fare(Zone::Airport, true) << " paise\n";
}
