// Task 6 - journeys at the peak boundaries (Middle zone, Rs 500 card)
// Peak windows are half-open: [08:00, 10:00) and [17:00, 20:00)
// Compile: g++ -std=c++17 -Wall -Wextra boundary.cpp -o boundary
#include <iomanip>
#include <iostream>
#include <string>

enum class Zone { Inner, Middle, Outer };
constexpr long long BASE_PAISE[] = {1000, 2000, 3000};
constexpr int PEAK_SURCHARGE_PERCENT = 20;
constexpr long long MIN_BALANCE_PAISE = 5000;

bool isPeak(int minute)
{
    return (minute >= 8 * 60 && minute < 10 * 60) || (minute >= 17 * 60 && minute < 20 * 60);
}

long long fare(Zone z, bool peak)
{
    long long base = 0;
    switch (z) {
    case Zone::Inner:  base = BASE_PAISE[0]; break;
    case Zone::Middle: base = BASE_PAISE[1]; break;
    case Zone::Outer:  base = BASE_PAISE[2]; break;
    }
    return peak ? base * (100 + PEAK_SURCHARGE_PERCENT) / 100 : base;
}

std::string rupees(long long paise)
{
    std::string p = std::to_string(paise % 100);
    return "Rs " + std::to_string(paise / 100) + '.' + (p.size() < 2 ? "0" + p : p);
}

int main()
{
    long long balance = 50000;                       // Rs 500.00
    const int times[][2] = {{7, 59}, {8, 0}, {9, 59}, {10, 0}, {17, 0}, {19, 59}, {20, 0}};
    std::cout << "Time   Peak  Fare      Balance\n";
    for (const auto& t : times) {
        int minute = t[0] * 60 + t[1];
        bool peak = isPeak(minute);
        long long f = fare(Zone::Middle, peak);
        if (balance - f < MIN_BALANCE_PAISE) { std::cout << "denied\n"; continue; }
        balance -= f;
        std::cout << std::setfill('0') << std::setw(2) << t[0] << ':' << std::setw(2) << t[1]
                  << std::setfill(' ') << "  " << std::left << std::setw(6) << (peak ? "yes" : "no")
                  << std::right << rupees(f) << "  " << rupees(balance) << '\n';
    }
}
