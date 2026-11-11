// Benchmarks used in the alternative-concept analyses (Tasks 5, 6, 8 and 10)
// Compile: g++ -std=c++17 -Wall -Wextra -O2 benchmarks.cpp -o benchmarks
// Timings depend on the machine; compare the ratios, not the absolute values.
#include <array>
#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using Clock = std::chrono::steady_clock;
template <class F>
double nsPer(long long n, F f)
{
    auto t0 = Clock::now();
    f();
    auto t1 = Clock::now();
    return std::chrono::duration<double, std::nano>(t1 - t0).count() / n;
}

// --- Task 5: peak check, expression vs lookup table ---
bool isPeakExpr(int m) { return (m >= 480 && m < 600) || (m >= 1020 && m < 1200); }
constexpr std::array<bool, 24> PEAK_HOUR{0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0,
                                         0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0};
bool isPeakTable(int m) { return PEAK_HOUR[m / 60]; }

// --- Task 6: base fare, switch vs array ---
enum class Zone { Inner, Middle, Outer };
long long fareSwitch(Zone z)
{
    switch (z) {
    case Zone::Inner:  return 1000;
    case Zone::Middle: return 2000;
    case Zone::Outer:  return 3000;
    }
    return 0;
}
constexpr long long FARE[] = {1000, 2000, 3000};
long long fareArray(Zone z) { return FARE[static_cast<int>(z)]; }

int main()
{
    const long long N = 60'000'000;
    // inputs prepared in advance so the loops time only the check itself
    const int K = 1 << 20;
    std::vector<int> minute(K);
    std::vector<Zone> zone(K);
    unsigned x = 12345;
    for (int i = 0; i < K; ++i) {
        x = x * 1103515245u + 12345u;               // simple pseudo-random sequence
        minute[i] = static_cast<int>((x >> 8) % 1440);
        zone[i] = static_cast<Zone>((x >> 4) % 3);
    }
    long long sink = 0;

    std::cout << "--- Task 5: peak check (" << N << " checks) ---\n";
    double e = nsPer(N, [&] { for (long long i = 0; i < N; ++i) sink += isPeakExpr(minute[i & (K - 1)]); });
    double t = nsPer(N, [&] { for (long long i = 0; i < N; ++i) sink += isPeakTable(minute[i & (K - 1)]); });
    std::cout << "  expression " << e << " ns, table " << t << " ns per check, sizeof(table) = "
              << sizeof(PEAK_HOUR) << " bytes\n";

    std::cout << "--- Task 6: base fare lookup (" << N << " lookups) ---\n";
    double s = nsPer(N, [&] { for (long long i = 0; i < N; ++i) sink += fareSwitch(zone[i & (K - 1)]); });
    double a = nsPer(N, [&] { for (long long i = 0; i < N; ++i) sink += fareArray(zone[i & (K - 1)]); });
    std::cout << "  switch " << s << " ns, array " << a << " ns per lookup, sizeof(array) = "
              << sizeof(FARE) << " bytes\n";

    std::cout << "--- Task 8: reading 1,000,000 short lines ---\n";
    std::string text;
    for (int i = 0; i < 1'000'000; ++i) text += std::to_string(i % 24) + '\n';
    const long long L = 1'000'000;
    double cinTime = nsPer(L, [&] {
        std::istringstream in(text);
        int v;
        while (in >> v) sink += v;
    });
    double glTime = nsPer(L, [&] {
        std::istringstream in(text);
        std::string line;
        while (std::getline(in, line)) {
            std::size_t used = 0;
            sink += std::stoi(line, &used);
        }
    });
    std::cout << "  >> " << cinTime << " ns, getline + stoi " << glTime << " ns per value\n";

    std::cout << "--- Task 10: one addition into the day total (" << N << " additions) ---\n";
    volatile int fareStep = 750;
    int ti = 0; long long tl = 0; double td = 0.0;
    double ni = nsPer(N, [&] { for (long long i = 0; i < N; ++i) ti += fareStep; });
    double nl = nsPer(N, [&] { for (long long i = 0; i < N; ++i) tl += fareStep; });
    double nd = nsPer(N, [&] { for (long long i = 0; i < N; ++i) td += fareStep / 100.0; });
    std::cout << "  int " << ni << " ns, long long " << nl << " ns, double " << nd << " ns per addition\n";
    std::cout << "  sizes: int " << sizeof(int) << ", long long " << sizeof(long long) << ", double "
              << sizeof(double) << " bytes\n";

    std::cout << "(checksum " << (sink + ti + tl + static_cast<long long>(td)) % 1000 << ")\n";
}
