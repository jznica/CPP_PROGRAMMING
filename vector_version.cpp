// Q2 alternative: std::vector<std::string> -> Rule of Zero (compiler-generated copy is deep).
// Compile: g++ -std=c++17 -Wall -Wextra -O2 vector_version.cpp -o vector_version
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

class Prescription {
    std::vector<std::string> medicines;   // owns its memory: no copy ctor/assignment/destructor needed
public:
    explicit Prescription(int count) : medicines(count) {}
    void set(int i, const std::string& m) { medicines[i] = m; }
    std::string get(int i) const { return medicines[i]; }
    int size() const { return static_cast<int>(medicines.size()); }
};

// A deliberately SHALLOW "copy" for comparison: just shares a pointer (O(1)).
struct SharedView { const std::string* data; int n; };

int main() {
    Prescription p(2);
    p.set(0, "Paracetamol");
    p.set(1, "ORS");
    Prescription q = p;
    q.set(1, "Cetirizine");
    Prescription r(1);
    r = p;
    p.set(0, "Ibuprofen");
    std::cout << p.get(0) << ' ' << p.get(1) << " | " << q.get(0) << ' ' << q.get(1)
              << " | " << r.get(0) << ' ' << r.get(1) << '\n';

    std::cout << "Time to copy (vector version / shallow pointer copy):\n";
    for (int n : {10, 1000, 100000}) {
        Prescription big(n);
        for (int i = 0; i < n; ++i) big.set(i, "Medicine" + std::to_string(i));
        int reps = n <= 1000 ? 10000 : 100;
        long long sink = 0;
        auto t0 = std::chrono::steady_clock::now();
        for (int k = 0; k < reps; ++k) { Prescription c = big; sink += c.size(); }
        auto t1 = std::chrono::steady_clock::now();
        std::vector<std::string> raw(n);
        auto t2 = std::chrono::steady_clock::now();
        for (int k = 0; k < reps; ++k) { volatile SharedView v{raw.data(), n}; sink += v.n; }
        auto t3 = std::chrono::steady_clock::now();
        std::cout << "  n = " << n << "\t: vector deep copy "
                  << std::chrono::duration<double, std::micro>(t1 - t0).count() / reps
                  << " us,  shallow copy "
                  << std::chrono::duration<double, std::nano>(t3 - t2).count() / reps << " ns"
                  << (sink < 0 ? "!" : "") << '\n';
    }
}
