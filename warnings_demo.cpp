// Task 10 - patterns collected from the first gate version, compiled with -Wall -Wextra
#include <iostream>
#include <vector>
enum class Zone { Inner, Middle, Outer };

long long oldFare(Zone z)
{
    long long base = 0;
    switch (z) {                       // Outer forgotten -> 0 paise
    case Zone::Inner:  base = 1000; break;
    case Zone::Middle: base = 2000; break;
    }
    return base;
}

int main(int argc, char* argv[])
{
    std::vector<long long> fares = {1000, 2000, 3000};
    for (int i = 0; i < fares.size(); ++i)   // int compared with size_t
        std::cout << fares[i] << ' ';
    long long balance = argc;
    if (balance = 0)                          // assignment, not comparison
        std::cout << "empty card";
    int unusedTotal = 0;                      // dead code: a forgotten total
    std::cout << oldFare(Zone::Outer) << '\n';
}
