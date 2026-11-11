// Task 3 - Data types, sizes, ranges, and why money is stored in integer paise
// Compile: g++ -std=c++17 -Wall -Wextra types_demo.cpp -o types_demo
#include <climits>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

template <class T>
void row(const std::string& name, const std::string& use)
{
    std::cout << std::left << std::setw(10) << name << std::right << std::setw(5) << sizeof(T)
              << std::setw(24) << static_cast<double>(std::numeric_limits<T>::lowest())
              << std::setw(25) << static_cast<double>(std::numeric_limits<T>::max())
              << "  " << use << '\n';
}

int main()
{
    // 1. sizeof and ranges on this system
    std::cout << "Type       size  min                      max                     used for\n";
    row<bool>("bool", "peak flag, canTravel result");
    row<char>("char", "passenger code R/S/C");
    row<int>("int", "hour, minute-of-day, zone index");
    row<unsigned>("unsigned", "(avoided - mixes badly with int)");
    row<long long>("long long", "fares, balances, totals (paise)");
    row<float>("float", "(rejected for money)");
    row<double>("double", "averages / reports only");
    row<std::size_t>("size_t", "array index, journey count");

    // 2. is int enough for a day's collection across the network?
    const long long journeys = 3000000, maxFarePaise = 3600;   // Rs 30 + 20 % surcharge
    const long long worstDay = journeys * maxFarePaise;
    std::cout << "\nWorst-case network day = " << worstDay << " paise; INT_MAX = " << INT_MAX
              << (worstDay > INT_MAX ? " -> int is NOT enough\n" : " -> int is enough\n");

    // 3. precision problem 1: rupees (double) -> paise by a truncating cast
    int wrong = 0, firstWrong = -1, firstGot = 0;
    for (int paise = 0; paise < 100000; ++paise) {
        double rupees = paise / 100.0;
        long long truncated = static_cast<long long>(rupees * 100);
        if (truncated != paise) {
            if (firstWrong < 0) { firstWrong = paise; firstGot = static_cast<int>(truncated); }
            ++wrong;
        }
    }
    std::cout << "Truncating rupees*100 gives the wrong paise for " << wrong << " of 100000 amounts; first: Rs "
              << firstWrong / 100 << '.' << std::setw(2) << std::setfill('0') << firstWrong % 100
              << std::setfill(' ') << " -> " << firstGot << " paise\n";
    // resolution: round once, then stay in integer paise
    //   long long paise = std::llround(rupees * 100);

    // 4. precision problem 2: adding Rs 0.10 n times as double rupees
    std::cout << "\n       n   double sum (Rs)        exact (Rs)    abs error (Rs)\n";
    for (long long n : {10LL, 1000LL, 20000LL, 500000LL, 3000000LL}) {
        double sum = 0.0;
        long long paise = 0;
        for (long long i = 0; i < n; ++i) { sum += 0.10; paise += 10; }
        const double exact = paise / 100.0;
        std::cout << std::setw(8) << n << std::fixed << std::setprecision(10) << std::setw(23) << sum
                  << std::setprecision(2) << std::setw(17) << exact << "  " << std::scientific
                  << std::setprecision(3) << std::fabs(sum - exact) << '\n' << std::defaultfloat;
    }
}
