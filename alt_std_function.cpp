// A3 alternative - payers as std::function / function pointers instead of an abstract Payer.
// Measures the space per payer and the time per call for both designs.
// Build: g++ -std=c++17 -O2 -Wall -Wextra alt_std_function.cpp -o alt_std_function
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <functional>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <vector>

struct Bill { double total; std::string package; };

// ---- design 1: abstract class (data members hold the policy parameters)
struct Payer { virtual ~Payer() = default; virtual double amountCovered(const Bill&) const = 0; };
struct SelfPay : Payer { double amountCovered(const Bill&) const override { return 0; } };
struct Insurance : Payer {
    double cover, copay;
    Insurance(double c, double p) : cover(c), copay(p) {}
    double amountCovered(const Bill& b) const override { return std::min(b.total * (100 - copay) / 100, cover); }
};
struct Scheme : Payer {
    std::map<std::string, double> rates;
    explicit Scheme(std::map<std::string, double> r) : rates(std::move(r)) {}
    double amountCovered(const Bill& b) const override
    {
        auto it = rates.find(b.package);
        return it == rates.end() ? 0 : std::min(it->second, b.total);
    }
};

// ---- design 2: policy function chosen at run time (captured state holds the parameters)
using Policy = std::function<double(const Bill&)>;
double selfPayFn(const Bill&) { return 0; }   // plain function pointer: no state possible

int main()
{
    double cover = 50000, copay = 10;
    std::map<std::string, double> rates{{"OP", 1000}, {"IP", 30000}, {"DC-CATARACT", 15000}, {"ER", 5000}};

    Policy self = selfPayFn;
    Policy ins = [cover, copay](const Bill& b) { return std::min(b.total * (100 - copay) / 100, cover); };
    Policy sch = [rates](const Bill& b) {
        auto it = rates.find(b.package);
        return it == rates.end() ? 0 : std::min(it->second, b.total);
    };
    double (*fp)(const Bill&) = selfPayFn;

    std::printf("Space per payer (bytes)\n");
    std::printf("  abstract class : SelfPay %zu, Insurance %zu, Scheme %zu (vptr + data members)\n",
                sizeof(SelfPay), sizeof(Insurance), sizeof(Scheme));
    std::printf("  std::function  : %zu each; the Insurance lambda captures %zu bytes (fits the small buffer);\n"
                "                   the Scheme lambda captures a %zu-byte map -> heap-allocated inside std::function\n",
                sizeof(Policy), 2 * sizeof(double), sizeof(rates));
    std::printf("  function ptr   : %zu (no place to store a cover limit -> needs globals or extra parameters)\n\n", sizeof(fp));

    Bill ip{52000, "IP"};
    Insurance insObj(cover, copay);
    Scheme schObj(rates);
    std::printf("Same answers: Insurance %.2f vs %.2f, Scheme %.2f vs %.2f, SelfPay %.2f vs %.2f\n\n",
                insObj.amountCovered(ip), ins(ip), schObj.amountCovered(ip), sch(ip), SelfPay().amountCovered(ip), self(ip));

    // time per call: 1,000,000 bills, payer chosen per bill at random
    const std::size_t N = 1000000;
    std::mt19937 g(5);
    std::uniform_int_distribution<int> pick(0, 1);
    std::uniform_real_distribution<double> amt(100, 60000);
    std::vector<Bill> bills(N);
    std::vector<int> which(N);
    for (std::size_t i = 0; i < N; ++i) { bills[i] = {amt(g), "OP"}; which[i] = pick(g); }
    std::unique_ptr<Payer> objs[2] = {std::make_unique<SelfPay>(), std::make_unique<Insurance>(cover, copay)};
    Policy fns[2] = {self, ins};

    auto med = [](auto f, double& out) {
        std::vector<double> t;
        for (int r = 0; r < 21; ++r) {
            auto a = std::chrono::steady_clock::now();
            out = f();
            auto b = std::chrono::steady_clock::now();
            t.push_back(std::chrono::duration<double, std::nano>(b - a).count());
        }
        std::sort(t.begin(), t.end());
        return t[10];
    };
    double s1 = 0, s2 = 0;
    double tv = med([&] { double s = 0; for (std::size_t i = 0; i < N; ++i) s += objs[which[i]]->amountCovered(bills[i]); return s; }, s1);
    double tf = med([&] { double s = 0; for (std::size_t i = 0; i < N; ++i) s += fns[which[i]](bills[i]); return s; }, s2);
    std::printf("Time per call (1,000,000 calls, -O2, median of 21)\n");
    std::printf("  virtual Payer::amountCovered : %.2f ns   sum %.2f\n", tv / N, s1);
    std::printf("  std::function call           : %.2f ns   sum %.2f\n", tf / N, s2);
}
