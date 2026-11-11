// Q5 timings: prefix vs postfix ++ and checked vs unchecked vs at() subscript
// Compile twice: g++ -std=c++17 -Wall -Wextra -O0 bench.cpp -o bench_O0
//                g++ -std=c++17 -Wall -Wextra -O2 bench.cpp -o bench_O2
#include <algorithm>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <vector>

class Time {
    int h, m, s;
public:
    Time(int hh = 0, int mm = 0, int ss = 0) : h(hh), m(mm), s(ss) {}
    Time& operator++() { if (++s == 60) { s = 0; if (++m == 60) { m = 0; ++h; } } return *this; }
    Time operator++(int) { Time old = *this; ++*this; return old; }
    long seconds() const { return h * 3600L + m * 60L + s; }
};

class Playlist {
    std::vector<int> len;
public:
    explicit Playlist(int n) : len(n) { for (int i = 0; i < n; ++i) len[i] = i % 300; }
    int size() const { return static_cast<int>(len.size()); }
    int& checked(int i) {                              // operator[] with range check
        static int bad = 0;
        if (i < 0 || i >= size()) { std::cerr << "out of range\n"; return bad; }
        return len[i];
    }
    int& at(int i) {                                   // throws std::out_of_range
        if (i < 0 || i >= size()) throw std::out_of_range("Playlist::at");
        return len[i];
    }
    int& unchecked(int i) { return len[i]; }           // like a built-in array
};

using Clock = std::chrono::steady_clock;
static double ms(Clock::time_point a, Clock::time_point b) {
    return std::chrono::duration<double, std::milli>(b - a).count();
}

template <class F>
double best(F f) {                       // best of 5 runs, removes warm-up noise
    double m = 1e18;
    for (int r = 0; r < 5; ++r) { auto a = Clock::now(); f(); auto b = Clock::now(); m = std::min(m, ms(a, b)); }
    return m;
}

int main() {
    const int N = 10000000;
    Time t1, t2;
    double pre = best([&] { for (int i = 0; i < N; ++i) ++t1; });
    double post = best([&] { for (int i = 0; i < N; ++i) t2++; });
    std::cout << "10,000,000 increments: prefix " << pre << " ms, postfix " << post
              << " ms  (check " << t1.seconds() << " == " << t2.seconds() << ")\n";

    Playlist p(1000);
    long long s1 = 0, s2 = 0, s3 = 0;
    double u = best([&] { for (int i = 0; i < N; ++i) s1 += p.unchecked(i % 1000); });
    double c = best([&] { for (int i = 0; i < N; ++i) s2 += p.checked(i % 1000); });
    double t = best([&] { for (int i = 0; i < N; ++i) s3 += p.at(i % 1000); });
    std::cout << "10,000,000 accesses: unchecked " << u << " ms, checked operator[] " << c
              << " ms, at() " << t << " ms  (sums equal: " << std::boolalpha
              << (s1 == s2 && s2 == s3) << ")\n";
    try { p.at(1000); } catch (const std::out_of_range& ex) { std::cout << "at(1000) threw std::out_of_range: " << ex.what() << '\n'; }
}
