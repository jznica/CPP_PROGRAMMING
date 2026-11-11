// Task 2 - Tokens, keywords, identifiers and constants
// One set of fare constants; #define vs constexpr; enum class vs int.
// Compile: g++ -std=c++17 -Wall -Wextra constants_demo.cpp -o constants_demo
// Error  : g++ -std=c++17 -Wall -Wextra -DSHOW_ERROR -c constants_demo.cpp
#include <array>
#include <cstddef>
#include <iostream>

// ---- the single set of fare constants (edit here only) ----
enum class Zone { Inner, Middle, Outer };              // scoped, no implicit int
constexpr std::array<long long, 3> BASE_PAISE{1000, 2000, 3000};
constexpr int PEAK_SURCHARGE_PERCENT = 20;
constexpr long long MIN_BALANCE_PAISE = 5000;

// fares must increase outward - checked by the compiler, not at run time
static_assert(BASE_PAISE[0] < BASE_PAISE[1] && BASE_PAISE[1] < BASE_PAISE[2],
              "fares must increase from Inner to Outer");

// ---- the same idea with a macro (no type, no scope, no parentheses) ----
#define MACRO_PEAK_FACTOR 100 + 20

constexpr long long peakFare(long long base)
{
    return base * (100 + PEAK_SURCHARGE_PERCENT) / 100;
}

int main()
{
    const long long inner = BASE_PAISE[static_cast<std::size_t>(Zone::Inner)];

    std::cout << "constexpr peak fare (Inner) = " << peakFare(inner) << " paise\n";
    // text substitution: inner * 100 + 20 / 100  ->  100000 + 0
    std::cout << "macro     peak fare (Inner) = " << inner * MACRO_PEAK_FACTOR / 100 << " paise\n";

    const auto outer = static_cast<std::size_t>(Zone::Outer);   // explicit cast needed
    std::cout << "Zone::Outer index = " << outer << ", fare = " << BASE_PAISE[outer] << " paise\n";
    std::cout << "Minimum balance = " << MIN_BALANCE_PAISE / 100 << " rupees\n";

#ifdef SHOW_ERROR
    // the next line must not compile
    int bad = Zone::Inner;            // error: enum class does not convert to int
    (void)bad;
#endif
}
