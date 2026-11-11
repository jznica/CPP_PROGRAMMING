// Task 7 - procedural version: global balance + free functions
// Compile: g++ -std=c++17 -Wall -Wextra procedural.cpp -o procedural
#include <iostream>

const long long BASE_PAISE[] = {1000, 2000, 3000};
const long long MIN_BALANCE_PAISE = 5000;

long long g_balance = 10000;          // any function can read or WRITE this

long long fare(int zone, bool peak)   // zone is a bare int: 7 is accepted silently
{
    long long base = BASE_PAISE[zone];
    return peak ? base * 120 / 100 : base;
}

bool canTravel(long long f) { return g_balance - f >= MIN_BALANCE_PAISE; }

void travel(int zone, bool peak)
{
    long long f = fare(zone, peak);
    if (canTravel(f)) g_balance -= f;
    std::cout << "travel  zone " << zone << " fare " << f << "  balance " << g_balance << '\n';
}

// Written later by another programmer: a "quick exit" gate that forgets the rule
void emergencyExitCharge(long long penalty) { g_balance -= penalty; }

int main()
{
    travel(0, false);                 // Inner, off-peak
    travel(2, true);                  // Outer, peak
    emergencyExitCharge(6000);        // bypasses the Rs 50 rule
    std::cout << "after emergency charge balance " << g_balance
              << (g_balance < 0 ? "   <-- NEGATIVE BALANCE" : "") << '\n';
}
