// Task 10 - day totals at 20,000 / 500,000 / 3,000,000 journeys in long long paise,
// double rupees, float rupees and int paise
// Compile: g++ -std=c++17 -Wall -Wextra -O2 totals.cpp -o totals
#include <array>
#include <climits>
#include <cstdint>
#include <iomanip>
#include <iostream>

// the fare mix: index i cycles through 3 zones x peak/off-peak x 3 passenger types
long long mixFare(int i)
{
    static const std::array<long long, 3> BASE{1000, 2000, 3000};
    static const std::array<long long, 3> CONCESSION{0, 25, 50};   // regular, student, senior
    const int zone = i % 3, peak = (i / 3) % 2, passenger = i / 6;
    const long long surcharge = peak ? 20 : 0;
    return (BASE[zone] * (100 + surcharge) * (100 - CONCESSION[passenger]) + 5000) / 10000;
}

int main()
{
    std::cout << "Journey mix: 18 combinations (3 zones x peak/off-peak x 3 passenger types), repeated\n\n";
    std::cout << "   Journeys  long long paise (Rs)         double rupees      float rupees  int paise (wrapped)\n";

    for (long long n : {20000LL, 500000LL, 3000000LL}) {
        long long paise = 0;
        double rupees = 0.0;
        float frupees = 0.0f;
        std::uint32_t wrapped = 0;          // what a 32-bit int would hold (no UB here)
        long long overflowAt = 0;
        for (long long j = 0; j < n; ++j) {
            long long f = mixFare(static_cast<int>(j % 18));
            paise += f;
            rupees += f / 100.0;
            frupees += f / 100.0f;
            wrapped += static_cast<std::uint32_t>(f);
            if (overflowAt == 0 && paise > INT_MAX) overflowAt = j + 1;
        }
        const double exact = paise / 100.0;
        std::cout << std::fixed << std::setw(11) << n << std::setprecision(2) << std::setw(22) << exact
                  << std::setprecision(6) << std::setw(22) << rupees << std::setprecision(2)
                  << std::setw(18) << frupees << std::setw(21) << static_cast<std::int32_t>(wrapped)
                  << '\n';
        std::cout << "             double - exact = " << std::scientific << std::setprecision(3)
                  << rupees - exact << " Rs,  float - exact = " << std::fixed << std::setprecision(2)
                  << frupees - exact << " Rs";
        if (overflowAt) std::cout << ",  int overflows at journey " << overflowAt;
        std::cout << '\n';
    }
}
