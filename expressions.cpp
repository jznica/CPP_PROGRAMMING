// Task 5 - peak check, surcharge and concession expressions; precedence demonstrations
// Compile: g++ -std=c++17 -Wall -Wextra expressions.cpp -o expressions
#include <array>
#include <iostream>
#include <string>

constexpr int MORNING_START = 8 * 60, MORNING_END = 10 * 60;   // [08:00, 10:00)
constexpr int EVENING_START = 17 * 60, EVENING_END = 20 * 60;  // [17:00, 20:00)
constexpr long long PEAK_SURCHARGE_PERCENT = 20;

bool isPeak(int minute)
{
    return (minute >= MORNING_START && minute < MORNING_END) ||
           (minute >= EVENING_START && minute < EVENING_END);
}

// alternative: lookup table indexed by the hour (works only for whole-hour boundaries)
constexpr std::array<bool, 24> PEAK_HOUR{0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0,
                                         0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0};
bool isPeakTable(int minute) { return PEAK_HOUR[minute / 60]; }

long long applyRules(long long base, bool peak, int concessionPercent)
{
    long long percent = 100 + (peak ? PEAK_SURCHARGE_PERCENT : 0);   // ?: binds looser than +
    return (base * percent * (100 - concessionPercent) + 5000) / 10000;
}

int main()
{
    std::cout << "time   expr  table\n";
    for (const char* t : {"07:59", "08:00", "09:59", "10:00", "16:59", "17:00", "19:59", "20:00"}) {
        int minute = std::stoi(std::string(t, 2)) * 60 + std::stoi(std::string(t + 3, 2));
        std::cout << t << "  " << isPeak(minute) << "     " << isPeakTable(minute) << '\n';
    }

    std::cout << "Outer peak regular  = " << applyRules(3000, true, 0) << " paise\n";
    std::cout << "Middle peak senior  = " << applyRules(2000, true, 50) << " paise\n";
    std::cout << "Inner off student   = " << applyRules(1000, false, 25) << " paise\n";

    long long balance = 9000;
    balance -= applyRules(3000, true, 0);           // compound assignment: right side first
    std::cout << "balance after -= : " << balance << '\n';

    // x++ vs ++x, and why i = i++ is a mistake:
    // x++ yields the old value, ++x the new one; i = i++ modifies i twice in one expression
    int i = 5;                                  // start value

    i = i++;                                    // C++17: defined, but i stays 5 (a no-op bug)
    std::cout << "i after i = i++ : " << i << '\n';
}
