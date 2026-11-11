// Q7 - Complex-number class from first principles : AC Circuit Impedance Calculator
// Compile: g++ -std=c++17 -Wall -Wextra -O2 impedance.cpp -o impedance && ./impedance circuit_input.txt
#include <chrono>
#include <cmath>
#include <complex>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

class Complex {
    double re, im;

public:
    Complex() : re(0.0), im(0.0) {}                    // default: 0 + j0
    Complex(double r) : re(r), im(0.0) {}              // from a real part (a resistor) - implicit on purpose
    Complex(double r, double i) : re(r), im(i) {}      // from real and imaginary parts

    double real() const { return re; }
    double imag() const { return im; }
    double magnitude() const { return std::hypot(re, im); }          // |z| without overflow in re*re
    double phaseDeg() const { return std::atan2(im, re) * 180.0 / M_PI; }

    // Compound assignment: members (they modify the left operand)
    Complex& operator+=(const Complex& z) { re += z.re; im += z.im; return *this; }
    Complex& operator-=(const Complex& z) { re -= z.re; im -= z.im; return *this; }
    Complex& operator*=(const Complex& z) {                          // (a+jb)(c+jd) = (ac-bd) + j(ad+bc)
        double r = re * z.re - im * z.im;
        im = re * z.im + im * z.re;
        re = r;
        return *this;
    }
    Complex& operator/=(const Complex& z) {                          // multiply by conjugate / |z|^2
        double d = z.re * z.re + z.im * z.im;
        if (d == 0.0) throw std::domain_error("division by a zero impedance (0 + j0)");
        double r = (re * z.re + im * z.im) / d;
        im = (im * z.re - re * z.im) / d;
        re = r;
        return *this;
    }

    // Binary operators: non-member friends -> symmetric, so 10 + z and z + 10 both work
    friend Complex operator+(Complex a, const Complex& b) { return a += b; }
    friend Complex operator-(Complex a, const Complex& b) { return a -= b; }
    friend Complex operator*(Complex a, const Complex& b) { return a *= b; }
    friend Complex operator/(Complex a, const Complex& b) { return a /= b; }
    friend bool operator==(const Complex& a, const Complex& b) { return a.re == b.re && a.im == b.im; }
    friend bool operator!=(const Complex& a, const Complex& b) { return !(a == b); }
    friend std::ostream& operator<<(std::ostream& os, const Complex& z) {
        os << z.re << (z.im < 0 || (z.im == 0 && std::signbit(z.im)) ? " - j" : " + j") << std::fabs(z.im);
        return os;
    }
};

// approximate comparison for checking against hand calculations
bool near(const Complex& a, const Complex& b, double tol = 1e-4) { return (a - b).magnitude() <= tol; }

// Component impedances
Complex resistor(double R) { return Complex(R); }
Complex inductor(double L, double w) { return Complex(0.0, w * L); }
Complex capacitor(double C, double w) {
    if (w * C == 0.0) throw std::domain_error("capacitor at w = 0 (DC) or C = 0 is an open circuit");
    return Complex(0.0, -1.0 / (w * C));
}
Complex parallel(const Complex& z1, const Complex& z2) { return (z1 * z2) / (z1 + z2); }

void report(const std::string& label, const Complex& z) {
    std::cout << "  " << std::left << std::setw(22) << label << std::right << "Z = " << z
              << " ohm   |Z| = " << z.magnitude() << " ohm   angle = " << z.phaseDeg() << " deg\n";
}

int main(int argc, char** argv) {
    double R = 10, L = 0.1, C = 100e-6, f = 50;
    std::ifstream in(argc > 1 ? argv[1] : "circuit_input.txt");
    if (in) in >> R >> L >> C >> f;
    const double w = 2 * M_PI * f;
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Inputs: R = " << R << " ohm, L = " << L << " H, C = " << C * 1e6 << " uF, f = " << f
              << " Hz, w = " << w << " rad/s\n\n";

    Complex ZR = resistor(R), ZL = inductor(L, w), ZC = capacitor(C, w);
    std::cout << "--- Component impedances ---\n";
    report("Resistor", ZR);
    report("Inductor", ZL);
    report("Capacitor", ZC);

    std::cout << "\n--- Series and parallel RLC ---\n";
    Complex series = ZR + ZL + ZC;
    Complex par = parallel(parallel(ZR, ZL), ZC);
    report("Series R+L+C", series);
    report("Parallel R||L||C", par);

    // Hand calculation (see report): series = R + j(wL - 1/(wC)); parallel via admittances
    double X = w * L - 1.0 / (w * C);
    Complex seriesHand(R, X);
    double G = 1.0 / R, B = w * C - 1.0 / (w * L);                 // Y = G + jB
    Complex parHand(G / (G * G + B * B), -B / (G * G + B * B));    // Z = 1/Y
    std::cout << std::boolalpha << "  series matches hand calculation   : " << near(series, seriesHand)
    << '\n'
              << "  parallel matches hand calculation : " << near(par, parHand) << '\n';

    std::cout << "\n--- Operator checks ---\n";
    Complex a(3, 4), b(1, -2);
    std::cout << "  a = " << a << ", b = " << b << '\n'
              << "  a + b = " << a + b << ",  a - b = " << a - b << '\n'
              << "  a * b = " << a * b << ",  a / b = " << a / b << '\n'
              << "  (a / b) * b == a ? " << ((a / b) * b == a) << "   near? " << near((a / b) * b, a) <<
     '\n'
              << "  10 + a = " << 10 + a << "  (needs + to be a non-member)\n"
              << "  |a| = " << a.magnitude() << '\n';

    std::cout << "\n--- Division by a zero impedance ---\n";
    try {
        Complex bad = a / Complex(0, 0);
        std::cout << "  result " << bad << '\n';
    } catch (const std::domain_error& e) {
        std::cout << "  caught std::domain_error: " << e.what() << '\n';
    }
    try {   // ideal parallel LC at resonance: ZL + ZC = j1 - j1 = 0
        Complex lc = parallel(inductor(1.0, 1.0), capacitor(1.0, 1.0));
        std::cout << "  LC tank " << lc << '\n';
    } catch (const std::domain_error& e) {
        std::cout << "  parallel LC at resonance (L = 1 H, C = 1 F, w = 1): " << e.what() << '\n';
    }
    try {
        capacitor(C, 0.0);
    } catch (const std::domain_error& e) {
        std::cout << "  " << e.what() << '\n';
    }

    std::cout << "\n--- Comparison with std::complex<double> ---\n";
    using SC = std::complex<double>;
    SC sR(R, 0), sL(0, w * L), sC(0, -1 / (w * C));
    SC sSeries = sR + sL + sC;
    SC sPar = (sR * sL / (sR + sL)) * sC / ((sR * sL / (sR + sL)) + sC);
    std::cout << std::scientific << std::setprecision(3)
              << "  |series - std|   = " << std::abs(SC(series.real(), series.imag()) - sSeries) << '\n'
              << "  |parallel - std| = " << std::abs(SC(par.real(), par.imag()) - sPar) << '\n';
    std::cout << "  std::complex (3+4j)/(0+0j) = " << SC(3, 4) / SC(0, 0) << "   (no exception: inf/nan)\n";
    Complex big(1e200, 1e200);
    std::cout << "  ours  (1e200+1e200j)/(1e200+1e200j) = " << big / big << "   <- overflow in c*c+d*d\n";
    std::cout << "  std:: (1e200+1e200j)/(1e200+1e200j) = " << SC(1e200, 1e200) / SC(1e200, 1e200) << '\n';
    std::cout << "  sizeof(Complex) = " << sizeof(Complex) << ", sizeof(std::complex<double>) = " << sizeof(SC) << '\n';

    std::cout << std::fixed << std::setprecision(2) << "\n--- Time per operation (10,000,000 ops) ---\n";
    const int N = 10000000;
    volatile double vr = 1.0001, vi = 0.9999;
    Complex acc(1, 1);
    SC sacc(1, 1);
    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < N; ++i) { Complex z(vr, vi); acc = acc * z / z + z; }
    auto t1 = std::chrono::steady_clock::now();
    for (int i = 0; i < N; ++i) { SC z(vr, vi); sacc = sacc * z / z + z; }
    auto t2 = std::chrono::steady_clock::now();
    auto ns = [&](auto p, auto q) { return std::chrono::duration<double, std::nano>(q - p).count() / N;
    };
    std::cout << "  ours        : " << ns(t0, t1) << " ns per (*, /, +)   result " << acc << '\n'
              << "  std::complex: " << ns(t1, t2) << " ns per (*, /, +)   result " << sacc << '\n';
}
