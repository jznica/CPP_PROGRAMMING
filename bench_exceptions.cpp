// B3.3 (+ A4 alt and B2 alt) - cost of reporting errors while validating 1,000,000 bills
//   return code : enum Status + out parameter                  (A4 alternative)
//   optional    : std::optional<double>, empty = rejected       (B2 alternative)
//   exception   : throw InvalidChargeError / catch per bill     (chosen design)
// with 0%, 1% and 50% invalid bills (a negative charge), placed at random.
// Build: g++ -std=c++17 -O2 -Wall -Wextra bench_exceptions.cpp -o bench_exceptions
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

struct Rec {
    double consultation, tests;
};

class InvalidChargeError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

enum class Status { Ok, NegativeCharge };

// noinline: stop the optimiser from merging validation into the loop, so every design
// pays one real function call per bill (as it would across translation units).
__attribute__((noinline)) Status validateRC(const Rec& r, double& out) noexcept
{
    if (r.consultation < 0 || r.tests < 0) return Status::NegativeCharge;
    out = r.consultation + r.tests;
    return Status::Ok;
}
__attribute__((noinline)) std::optional<double> validateOpt(const Rec& r) noexcept
{
    if (r.consultation < 0 || r.tests < 0) return std::nullopt;
    return r.consultation + r.tests;
}
__attribute__((noinline)) double validateEx(const Rec& r)
{
    if (r.consultation < 0 || r.tests < 0) throw InvalidChargeError("negative charge");
    return r.consultation + r.tests;
}

struct Result {
    double sum = 0;
    long rejected = 0;
};

template <class F>
double medianMs(F&& f, int reps, Result& out)
{
    std::vector<double> t;
    for (int i = 0; i < reps; ++i) {
        auto a = std::chrono::steady_clock::now();
        out = f();
        auto b = std::chrono::steady_clock::now();
        t.push_back(std::chrono::duration<double, std::milli>(b - a).count());
    }
    std::sort(t.begin(), t.end());
    return t[t.size() / 2];
}

int main()
{
    const std::size_t N = 1000000;
    const int reps = 21;
    std::printf("B3.3  validating %zu bills, -O2, median of %d runs\n", N, reps);
    std::printf("sizeof(Status)+sizeof(double) = %zu+%zu, sizeof(std::optional<double>) = %zu, sizeof(InvalidChargeError) = %zu\n\n",
                sizeof(Status), sizeof(double), sizeof(std::optional<double>), sizeof(InvalidChargeError));
    std::printf("%8s | %-11s | %10s | %8s | %9s | %s\n", "invalid", "design", "total ms", "ns/bill", "rejected", "extra cost per invalid bill");
    for (double pct : {0.0, 1.0, 50.0}) {
        std::mt19937 g(2026);
        std::uniform_real_distribution<double> fee(100, 5000), u(0, 100);
        std::vector<Rec> v(N);
        for (auto& r : v) r = {u(g) < pct ? -fee(g) : fee(g), fee(g)};

        Result rc, op, ex;
        double tRC = medianMs([&] {
            Result r;
            for (const auto& b : v) {
                double t;
                if (validateRC(b, t) == Status::Ok) r.sum += t; else ++r.rejected;
            }
            return r; }, reps, rc);
        double tOpt = medianMs([&] {
            Result r;
            for (const auto& b : v) {
                if (auto t = validateOpt(b)) r.sum += *t; else ++r.rejected;
            }
            return r; }, reps, op);
        double tEx = medianMs([&] {
            Result r;
            for (const auto& b : v) {
                try { r.sum += validateEx(b); }
                catch (const InvalidChargeError&) { ++r.rejected; }
            }
            return r; }, reps, ex);
        bool same = rc.sum == op.sum && op.sum == ex.sum && rc.rejected == op.rejected && op.rejected == ex.rejected;
        auto line = [&](const char* d, double ms, const Result& r, double base) {
            char extra[64] = "-";
            if (r.rejected > 0 && base >= 0)
                std::snprintf(extra, sizeof extra, "%.3f us", (ms - base) * 1000.0 / double(r.rejected));
            std::printf("%7.0f%% | %-11s | %10.2f | %8.2f | %9ld | %s\n", pct, d, ms, ms * 1e6 / double(N), r.rejected, extra);
        };
        line("return code", tRC, rc, -1);
        line("optional", tOpt, op, -1);
        line("exception", tEx, ex, tRC);
        std::printf("         results identical across designs: %s\n", same ? "yes" : "NO");
        std::printf("---------+-------------+------------+----------+-----------+----------------------------\n");
    }
}
