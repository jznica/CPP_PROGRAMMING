// B4 alternative - bill types loaded from a configuration file (rules.cfg) and
// interpreted at run time, compared with one virtual call per bill.
// Build: g++ -std=c++17 -O2 -Wall -Wextra alt_plugin_rules.cpp -o alt_plugin_rules
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// ---------- rule engine
struct Factor { int field; double constant; };              // field == -1 -> constant
using Term = std::vector<Factor>;                          // product of factors
struct Rule { std::string type; std::vector<Term> terms; };  // sum of terms

std::vector<Rule> loadRules(const std::string& path)
{
    std::ifstream in(path);
    if (!in) throw std::runtime_error("cannot open " + path);
    std::vector<Rule> rules;
    for (std::string line; std::getline(in, line);) {
        if (line.empty() || line[0] == '#') continue;
        auto eq = line.find('=');
        if (eq == std::string::npos) throw std::runtime_error("bad rule: " + line);
        Rule r;
        std::istringstream name(line.substr(0, eq));
        name >> r.type;
        std::string rhs = line.substr(eq + 1);
        rhs.erase(std::remove(rhs.begin(), rhs.end(), ' '), rhs.end());
        std::stringstream terms(rhs);
        for (std::string t; std::getline(terms, t, '+');) {
            Term term;
            std::stringstream fs(t);
            for (std::string f; std::getline(fs, f, '*');) {
                if (f.size() > 1 && f[0] == 'f') {
                    int idx = std::stoi(f.substr(1));
                    if (idx < 0 || idx > 3) throw std::runtime_error("field out of range in: " + line);
                    term.push_back({idx, 0});
                } else term.push_back({-1, std::stod(f)});
            }
            r.terms.push_back(term);
        }
        rules.push_back(r);
    }
    return rules;
}

struct DataBill { int rule; double f[4]; };   // rule index + up to 4 fields

inline double evaluate(const Rule& r, const DataBill& b)
{
    double sum = 0;
    for (const Term& t : r.terms) {
        double p = 1;
        for (const Factor& x : t) p *= x.field >= 0 ? b.f[x.field] : x.constant;
        sum += p;
    }
    return sum;
}

// ---------- compiled classes (virtual)
struct Bill { virtual ~Bill() = default; virtual double total() const noexcept = 0; };
struct OP final : Bill { double a, b; OP(double x, double y) : a(x), b(y) {} double total() const noexcept override { return a + b; } };
struct IP final : Bill { double d, r, p, m; IP(double w, double x, double y, double z) : d(w), r(x), p(y), m(z) {} double total() const noexcept override { return d * r + p + m; } };
struct DC final : Bill { double a; explicit DC(double x) : a(x) {} double total() const noexcept override { return a; } };
struct ER final : Bill { double a, h, r; ER(double x, double y, double z) : a(x), h(y), r(z) {} double total() const noexcept override { return a + h * r; } };
struct TELE final : Bill { double m, r, f; TELE(double x, double y, double z) : m(x), r(y), f(z) {} double total() const noexcept override { return m * r + f; } };

template <class F>
double medianNs(F&& f, int reps, double& out)
{
    std::vector<double> t;
    for (int i = 0; i < reps; ++i) {
        auto a = std::chrono::steady_clock::now();
        out = f();
        auto b = std::chrono::steady_clock::now();
        t.push_back(std::chrono::duration<double, std::nano>(b - a).count());
    }
    std::sort(t.begin(), t.end());
    return t[t.size() / 2];
}

int main()
{
    auto rules = loadRules("rules.cfg");
    std::size_t ruleBytes = 0;
    std::printf("loaded %zu rules from rules.cfg:\n", rules.size());
    for (auto& r : rules) {
        std::size_t factors = 0;
        for (auto& t : r.terms) factors += t.size();
        ruleBytes += sizeof(Rule) + r.type.capacity() + r.terms.size() * sizeof(Term) + factors * sizeof(Factor);
        std::printf("  %-4s : %zu terms, %zu factors\n", r.type.c_str(), r.terms.size(), factors);
    }
    std::printf("approx. memory for all rules: %zu bytes (shared by all bills)\n", ruleBytes);
    std::printf("per bill: DataBill = %zu bytes; virtual objects = 24..40 bytes + 8-byte pointer\n\n", sizeof(DataBill));

    // sanity check against the hand-verified bills of Task B1/B4
    DataBill tele{4, {20, 30, 99, 0}}, ip{1, {3, 2500, 40000, 4500}};
    std::printf("check: TELE(20 min x 30 + 99) = %.2f (expected 699.00), IP = %.2f (expected 52000.00)\n\n",
                evaluate(rules[4], tele), evaluate(rules[1], ip));

    const std::size_t N = 1000000;
    std::mt19937 g(11);
    std::uniform_int_distribution<int> k(0, 4);
    std::uniform_real_distribution<double> v(1, 5000);
    std::vector<DataBill> data(N);
    std::vector<std::unique_ptr<Bill>> objs;
    objs.reserve(N);
    for (auto& b : data) {
        b.rule = k(g);
        for (double& x : b.f) x = v(g);
        switch (b.rule) {
        case 0: objs.push_back(std::make_unique<OP>(b.f[0], b.f[1])); break;
        case 1: objs.push_back(std::make_unique<IP>(b.f[0], b.f[1], b.f[2], b.f[3])); break;
        case 2: objs.push_back(std::make_unique<DC>(b.f[0])); break;
        case 3: objs.push_back(std::make_unique<ER>(b.f[0], b.f[1], b.f[2])); break;
        default: objs.push_back(std::make_unique<TELE>(b.f[0], b.f[1], b.f[2])); break;
        }
    }
    double rv = 0, ri = 0;
    double tv = medianNs([&] { double s = 0; for (auto& b : objs) s += b->total(); return s; }, 21, rv);
    double ti = medianNs([&] { double s = 0; for (auto& b : data) s += evaluate(rules[b.rule], b); return s; }, 21, ri);
    std::printf("%zu bills, -O2, median of 21 runs\n", N);
    std::printf("  virtual call      : %7.2f ms  %6.2f ns/bill  total %.2f\n", tv / 1e6, tv / N, rv);
    std::printf("  rule interpreter  : %7.2f ms  %6.2f ns/bill  total %.2f\n", ti / 1e6, ti / N, ri);
    std::printf("  totals agree: %s, interpreter / virtual = %.2fx\n", std::fabs(rv - ri) <= 1e-9 * rv ? "yes" : "NO", ti / tv);
    std::printf("  200,000 bills/day cost: virtual %.3f ms, interpreter %.3f ms\n", tv / N * 200000 / 1e6, ti / N * 200000 / 1e6);
}
