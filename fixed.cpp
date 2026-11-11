// Task 4 - the same program with every conversion written explicitly
// Compile: g++ -std=c++17 -Wall -Wextra fixed.cpp -o fixed            (no warnings)
// Error  : g++ -std=c++17 -Wall -Wextra -DSHOW_ERROR -c fixed.cpp     (narrowing caught)
#include <iostream>
int main()
{
    int a{7}, b{2};
    double c{static_cast<double>(a / b)};          // integer division is now visibly intended
    double d{static_cast<double>(a) / b};          // real division
    std::cout << c << ' ' << d << '\n';

    char ch{static_cast<char>('A' + 2)};
    std::cout << ch << ' ' << static_cast<int>(ch + 1) << ' '
              << static_cast<char>(ch + 1) << '\n';

    bool flag{5 != 0};                               // say what "true" means
    std::cout << std::boolalpha << flag << '\n';

    unsigned int u{1};
    int s{-1};
    // compare in a signed type wide enough for both ranges
    bool less = static_cast<long long>(s) < static_cast<long long>(u);
    std::cout << (less ? "s < u" : "s >= u") << '\n';
    // the fare path: percent surcharge on paise, integer only
    long long base{3000};
    long long peakFare{base * (100 + 20) / 100};
    std::cout << "peak fare = " << peakFare << " paise\n";
#ifdef SHOW_ERROR
    int x{3.7};                               // narrowing double -> int: compile error with {}
    long long fare{36.5 * 100};               // narrowing double -> long long: compile error
#endif
}
