// Task 6 - Alternative: single Vehicle class with a type code and a switch in charge()
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

enum Kind { BICYCLE, TWO_WHEELER, CAR, ELECTRIC_CAR };
struct SwitchVehicle {
    Kind kind; std::string reg; double rate;
    double level = 0; int dd = 0, mm = 0, yy = 0;    // unused by bicycles
    int seats = 0;                                   // used only by cars
    double batteryKWh = 0;                           // used only by electric cars
    double charge(int days, double usage) const
    {
        switch (kind) {
        case BICYCLE:      return days * rate;
        case TWO_WHEELER:  return days * rate + usage * 105;
        case CAR:          return days * rate + (seats > 5 ? days * 300.0 : 0) + usage * 105;
        case ELECTRIC_CAR: return days * rate + (seats > 5 ? days * 300.0 : 0) + usage * 12;
        }
        return 0;                                     // a new kind needs a new case HERE
    }
};

struct V { std::string reg; double rate; virtual ~V() = default; virtual double charge(int, double) const = 0; };
struct Bi : V { double charge(int d, double) const override { return d * rate; } };
struct Tw : V { double level; int dd, mm, yy; double charge(int d, double u) const override { return d * rate + u * 105; } };
struct Ca : Tw { int seats; double charge(int d, double u) const override { return d * rate + (seats > 5 ? d * 300.0 : 0) + u * 105; } };
struct Ev : Ca { double kwh; double charge(int d, double u) const override { return d * rate + (seats > 5 ? d * 300.0 : 0) + u * 12; } };

int main()
{
    std::cout << "sizeof(SwitchVehicle) = " << sizeof(SwitchVehicle) << " (same for every kind)\n";
    std::cout << "sizeof Bicycle/TwoWheeler/Car/ElectricCar (virtual) = " << sizeof(Bi) << '/'
              << sizeof(Tw) << '/' << sizeof(Ca) << '/' << sizeof(Ev) << '\n';

    const int N = 4000; const int REPS = 5000;
    std::vector<SwitchVehicle> sv; std::vector<std::unique_ptr<V>> vv;
    for (int i = 0; i < N; ++i) {
        SwitchVehicle s{Kind(i % 4), "R", 100.0 + i % 7}; s.seats = 7; sv.push_back(s);
        std::unique_ptr<V> p;
        switch (i % 4) { case 0: p = std::make_unique<Bi>(); break; case 1: p = std::make_unique<Tw>(); break;
                         case 2: { auto c = std::make_unique<Ca>(); c->seats = 7; p = std::move(c); break; }
                         default: { auto e = std::make_unique<Ev>(); e->seats = 7; p = std::move(e); } }
        p->rate = 100.0 + i % 7; vv.push_back(std::move(p));
    }
    using C = std::chrono::steady_clock;
    volatile double sink = 0;
    auto t0 = C::now();
    for (int r = 0; r < REPS; ++r) for (const auto& s : sv) sink = sink + s.charge(3, 2.0);
    auto t1 = C::now();
    for (int r = 0; r < REPS; ++r) for (const auto& p : vv) sink = sink + p->charge(3, 2.0);
    auto t2 = C::now();
    double ns1 = std::chrono::duration<double, std::nano>(t1 - t0).count() / (double(N) * REPS);
    double ns2 = std::chrono::duration<double, std::nano>(t2 - t1).count() / (double(N) * REPS);
    std::cout << "avg time per charge(): switch " << ns1 << " ns, virtual " << ns2 << " ns  ("
              << N * REPS / 1000000 << " million calls each, -O2)\n";
}
