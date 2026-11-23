// B3.1, B3.2 and alternatives - cost of dispatch for n bills.
//   (a) virtual   : vector<unique_ptr<Bill>>, one virtual call per bill        (A1/B3)
//   (b) switch    : one struct with a type field, switch in total()          (A1 alt, B3)
//   (c) variant   : vector<std::variant<OP,IP,DC,ER>> + std::visit           (B3 alt)
//   (d) dyncast   : non-virtual total(), type found by up to k dynamic_casts (A2 alt)
//   (e) templates : one vector per type, compile-time polymorphism           (B1 alt)
// Bill types appear in random order (worst case for the branch predictor).
// Build: g++ -std=c++17 -O2 -Wall -Wextra bench_dispatch.cpp -o bench_dispatch
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <memory>
#include <random>
#include <variant>
#include <vector>

// ---------------- (a) virtual functions
struct Bill {
    virtual ~Bill() = default;
    virtual double total() const noexcept = 0;
};
struct OutPatientBill final : Bill {
    double c, t;
    OutPatientBill(double c_, double t_) : c(c_), t(t_) {}
    double total() const noexcept override { return c + t; }
};
struct InPatientBill final : Bill {
    int days;
    double perDay, proc, med;
    InPatientBill(int d, double r, double p, double m) : days(d), perDay(r), proc(p), med(m) {}
    double total() const noexcept override { return days * perDay + proc + med; }
};
struct DayCareBill final : Bill {
    double pkg;
    explicit DayCareBill(double p) : pkg(p) {}
    double total() const noexcept override { return pkg; }
};
struct EmergencyBill final : Bill {
    double base, hours, perHour;
    EmergencyBill(double b, double h, double r) : base(b), hours(h), perHour(r) {}
    double total() const noexcept override { return base + hours * perHour; }
};

// ---------------- (b) one struct + switch
enum class Kind : unsigned char { OP, IP, DC, ER };
struct BillRec {
    Kind kind;
    int days;
    double a, b, c;
    double total() const noexcept
    {
        switch (kind) {
        case Kind::OP: return a + b;
        case Kind::IP: return days * a + b + c;
        case Kind::DC: return a;
        case Kind::ER: return a + b * c;
        }
        return 0;
    }
};

// ---------------- (c) std::variant of plain (non-virtual) value types
struct OPv { double c, t; double total() const noexcept { return c + t; } };
struct IPv { int days; double perDay, proc, med; double total() const noexcept { return days * perDay + proc + med; } };
struct DCv { double pkg; double total() const noexcept { return pkg; } };
struct ERv { double base, hours, perHour; double total() const noexcept { return base + hours * perHour; } };
using BillVar = std::variant<OPv, IPv, DCv, ERv>;

// ---------------- (d) dynamic_cast chain (total() NOT virtual)
struct NBill { virtual ~NBill() = default; };   // polymorphic only for RTTI
struct NOP final : NBill { OPv d; double total() const noexcept { return d.total(); } };
struct NIP final : NBill { IPv d; double total() const noexcept { return d.total(); } };
struct NDC final : NBill { DCv d; double total() const noexcept { return d.total(); } };
struct NER final : NBill { ERv d; double total() const noexcept { return d.total(); } };
double totalByCast(const NBill* b)
{
    if (auto p = dynamic_cast<const NOP*>(b)) return p->total();   // 1 cast
    if (auto p = dynamic_cast<const NIP*>(b)) return p->total();   // 2 casts
    if (auto p = dynamic_cast<const NDC*>(b)) return p->total();   // 3 casts
    if (auto p = dynamic_cast<const NER*>(b)) return p->total();   // 4 casts
    return 0;
}

// ---------------- (e) templates: compile-time polymorphism, one container per type
template <class T>
double sumAll(const std::vector<T>& v)
{
    double s = 0;
    for (const T& x : v) s += x.total();   // statically bound, can be inlined
    return s;
}

// ---------------- data
struct Raw {
    Kind k;
    int days;
    double a, b, c;
};
std::vector<Raw> makeData(std::size_t n, unsigned seed)
{
    std::mt19937 g(seed);
    std::uniform_int_distribution<int> kind(0, 3), days(1, 15);
    std::uniform_real_distribution<double> fee(100, 5000), big(5000, 60000), hrs(0.5, 12);
    std::vector<Raw> r(n);
    for (auto& x : r) {
        x.k = static_cast<Kind>(kind(g));
        switch (x.k) {
        case Kind::OP: x = {x.k, 0, fee(g), fee(g), 0}; break;
        case Kind::IP: x = {x.k, days(g), fee(g), big(g), fee(g)}; break;
        case Kind::DC: x = {x.k, 0, big(g), 0, 0}; break;
        case Kind::ER: x = {x.k, 0, fee(g), hrs(g), fee(g)}; break;
        }
    }
    return r;
}

template <class F>
double timeIt(F&& f, int reps, double& result)   // median time (ns) of one pass
{
    std::vector<double> t;
    for (int r = 0; r < reps; ++r) {
        auto t0 = std::chrono::steady_clock::now();
        result = f();
        auto t1 = std::chrono::steady_clock::now();
        t.push_back(std::chrono::duration<double, std::nano>(t1 - t0).count());
    }
    std::sort(t.begin(), t.end());
    return t[t.size() / 2];
}

volatile double g_sink;

void correctness10()
{
    std::printf("B3.1  correctness on 10 bills (seed 7)\n");
    std::printf("  #  type  virtual total   switch total   agree\n");
    auto raw = makeData(10, 7);
    const char* nm[] = {"OP", "IP", "DC", "ER"};
    double sv = 0, ss = 0;
    int agree = 0;
    for (std::size_t i = 0; i < raw.size(); ++i) {
        const Raw& x = raw[i];
        std::unique_ptr<Bill> b;
        switch (x.k) {
        case Kind::OP: b = std::make_unique<OutPatientBill>(x.a, x.b); break;
        case Kind::IP: b = std::make_unique<InPatientBill>(x.days, x.a, x.b, x.c); break;
        case Kind::DC: b = std::make_unique<DayCareBill>(x.a); break;
        case Kind::ER: b = std::make_unique<EmergencyBill>(x.a, x.b, x.c); break;
        }
        BillRec rec{x.k, x.days, x.a, x.b, x.c};
        double v = b->total(), s = rec.total();
        sv += v;
        ss += s;
        agree += (v == s);
        std::printf("  %2zu  %s   %13.2f  %13.2f   %s\n", i + 1, nm[int(x.k)], v, s, v == s ? "yes" : "NO");
    }
    std::printf("  sum      %13.2f  %13.2f   %s   (%d/10 bills agree)\n\n", sv, ss, sv == ss ? "yes" : "NO", agree);
}

int main()
{
    correctness10();

    std::printf("Space per element (bytes): virtual object sizeof OP/IP/DC/ER = %zu/%zu/%zu/%zu + 8-byte pointer + heap header\n",
                sizeof(OutPatientBill), sizeof(InPatientBill), sizeof(DayCareBill), sizeof(EmergencyBill));
    std::printf("                           switch BillRec = %zu, variant = %zu (largest alternative IPv = %zu + tag + padding)\n\n",
                sizeof(BillRec), sizeof(BillVar), sizeof(IPv));

    std::printf("B3.2  time to total n bills, -O2, median of repetitions\n");
    std::printf("%11s | %-10s | %12s | %8s | %s\n", "n", "method", "ms per pass", "ns/bill", "total");
    for (std::size_t n : {std::size_t(1000), std::size_t(1000000), std::size_t(10000000)}) {
        auto raw = makeData(n, 42);
        int reps = n <= 1000 ? 20001 : (n <= 1000000 ? 21 : 7);
        int inner = n <= 1000 ? 1 : 1;
        (void)inner;
        double res = 0, ref = 0;
        auto row = [&](const char* name, double ns) {
            std::printf("%11zu | %-10s | %12.4f | %8.3f | %.2f%s\n", n, name, ns / 1e6, ns / double(n), res,
                        std::fabs(res - ref) <= 1e-9 * std::fabs(ref) ? "" : "  MISMATCH");
        };
        {   // (a) virtual
            std::vector<std::unique_ptr<Bill>> v;
            v.reserve(n);
            for (auto& x : raw) switch (x.k) {
                case Kind::OP: v.push_back(std::make_unique<OutPatientBill>(x.a, x.b)); break;
                case Kind::IP: v.push_back(std::make_unique<InPatientBill>(x.days, x.a, x.b, x.c)); break;
                case Kind::DC: v.push_back(std::make_unique<DayCareBill>(x.a)); break;
                case Kind::ER: v.push_back(std::make_unique<EmergencyBill>(x.a, x.b, x.c)); break;
                }
            double ns = timeIt([&] { double s = 0; for (const auto& b : v) s += b->total(); return s; }, reps, res);
            ref = res;
            row("virtual", ns);
        }
        {   // (b) switch
            std::vector<BillRec> v;
            v.reserve(n);
            for (auto& x : raw) v.push_back({x.k, x.days, x.a, x.b, x.c});
            double ns = timeIt([&] { double s = 0; for (const auto& b : v) s += b.total(); return s; }, reps, res);
            row("switch", ns);
        }
        {   // (c) variant
            std::vector<BillVar> v;
            v.reserve(n);
            for (auto& x : raw) switch (x.k) {
                case Kind::OP: v.emplace_back(OPv{x.a, x.b}); break;
                case Kind::IP: v.emplace_back(IPv{x.days, x.a, x.b, x.c}); break;
                case Kind::DC: v.emplace_back(DCv{x.a}); break;
                case Kind::ER: v.emplace_back(ERv{x.a, x.b, x.c}); break;
                }
            double ns = timeIt([&] {
                double s = 0;
                for (const auto& b : v) s += std::visit([](const auto& x) { return x.total(); }, b);
                return s; }, reps, res);
            row("variant", ns);
        }
        {   // (d) dynamic_cast chain
            std::vector<std::unique_ptr<NBill>> v;
            v.reserve(n);
            for (auto& x : raw) switch (x.k) {
                case Kind::OP: { auto p = std::make_unique<NOP>(); p->d = {x.a, x.b}; v.push_back(std::move(p)); break; }
                case Kind::IP: { auto p = std::make_unique<NIP>(); p->d = {x.days, x.a, x.b, x.c}; v.push_back(std::move(p)); break; }
                case Kind::DC: { auto p = std::make_unique<NDC>(); p->d = {x.a}; v.push_back(std::move(p)); break; }
                case Kind::ER: { auto p = std::make_unique<NER>(); p->d = {x.a, x.b, x.c}; v.push_back(std::move(p)); break; }
                }
            double ns = timeIt([&] { double s = 0; for (const auto& b : v) s += totalByCast(b.get()); return s; }, reps, res);
            row("dyncast", ns);
        }
        {   // (e) templates, one container per type (summation order differs -> compare with tolerance)
            std::vector<OPv> op; std::vector<IPv> ip; std::vector<DCv> dc; std::vector<ERv> er;
            for (auto& x : raw) switch (x.k) {
                case Kind::OP: op.push_back({x.a, x.b}); break;
                case Kind::IP: ip.push_back({x.days, x.a, x.b, x.c}); break;
                case Kind::DC: dc.push_back({x.a}); break;
                case Kind::ER: er.push_back({x.a, x.b, x.c}); break;
                }
            double ns = timeIt([&] { return sumAll(op) + sumAll(ip) + sumAll(dc) + sumAll(er); }, reps, res);
            row("templates", ns);
        }
        g_sink = res;
        std::printf("------------+------------+--------------+----------+------------\n");
    }
}
