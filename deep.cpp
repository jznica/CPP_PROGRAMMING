// Q2 - version 2: Rule of Three -> deep copy constructor, copy assignment, destructor.
// Compile: g++ -std=c++17 -Wall -Wextra -g -fsanitize=address deep.cpp -o deep_asan
//          g++ -std=c++17 -Wall -Wextra -O2 deep.cpp -o deep            (for timings)
#include <chrono>
#include <iostream>
#include <string>

class Prescription {
    int n;
    std::string* medicines;
public:
    static bool verbose;
    static int copyCount;

    explicit Prescription(int count) : n(count), medicines(new std::string[count]) {}

    // Copy constructor - DEEP copy: new array of the same size, copy every name
    Prescription(const Prescription& other) : n(other.n), medicines(new std::string[other.n]) {
        for (int i = 0; i < n; ++i) medicines[i] = other.medicines[i];
        ++copyCount;
        if (verbose) std::cout << "    [copy constructor called]\n";
    }

    // Copy assignment - self-assignment guard, allocate first, then release the old array
    Prescription& operator=(const Prescription& other) {
        if (verbose) std::cout << "    [copy assignment called]\n";
        if (this == &other) {
            if (verbose) std::cout << "    [self-assignment detected - nothing to do]\n";
            return *this;
        }
        std::string* fresh = new std::string[other.n];   // if this throws, *this is unchanged
        for (int i = 0; i < other.n; ++i) fresh[i] = other.medicines[i];
        delete[] medicines;
        medicines = fresh;
        n = other.n;
        return *this;
    }

    ~Prescription() { delete[] medicines; }

    void set(int i, const std::string& m) { medicines[i] = m; }
    std::string get(int i) const { return medicines[i]; }
    const void* buffer() const { return medicines; }
    int size() const { return n; }
};
bool Prescription::verbose = true;
int Prescription::copyCount = 0;

void show(const char* label, const Prescription& p) {
    std::cout << label << " [array at " << p.buffer() << "] : ";
    for (int i = 0; i < p.size(); ++i) std::cout << p.get(i) << (i + 1 < p.size() ? ", " : "\n");
}

int byValue(Prescription p) { return p.size(); }               // parameter is a COPY
int byConstRef(const Prescription& p) { return p.size(); }     // no copy
Prescription makeFollowUp() { Prescription p(1); p.set(0, "Zinc"); return p; }  // return by value

int main() {
    std::cout << "--- 1. Independence of copies ---\n";
    Prescription original(2);
    original.set(0, "Paracetamol");
    original.set(1, "ORS");
    Prescription followUp = original;          // copy constructor
    followUp.set(1, "Cetirizine");
    show("original ", original);
    show("follow-up", followUp);

    std::cout << "\n--- 2. Copies made when passing ---\n";
    Prescription::copyCount = 0;
    std::cout << "  pass by value:\n";
    byValue(original);
    std::cout << "  copies made = " << Prescription::copyCount << '\n';
    Prescription::copyCount = 0;
    std::cout << "  pass by const reference:\n";
    byConstRef(original);
    std::cout << "  copies made = " << Prescription::copyCount << '\n';
    Prescription::copyCount = 0;
    std::cout << "  return by value (copy elision):\n";
    Prescription z = makeFollowUp();
    std::cout << "  copies made = " << Prescription::copyCount << "  (" << z.get(0) << ")\n";

    std::cout << "\n--- 3. Copy assignment and self-assignment ---\n";
    Prescription r(1);
    r = original;
    show("r        ", r);
    Prescription& alias = r;
    r = alias;                                 // p = p
    show("r (after r = r)", r);

    std::cout << "\n--- 4. Time to copy a prescription ---\n";
    Prescription::verbose = false;
    for (int n : {10, 1000, 100000}) {
        Prescription big(n);
        for (int i = 0; i < n; ++i) big.set(i, "Medicine" + std::to_string(i));
        int reps = n <= 1000 ? 10000 : 100;
        long long sink = 0;
        auto t0 = std::chrono::steady_clock::now();
        for (int k = 0; k < reps; ++k) { Prescription c = big; sink += c.size(); }
        auto t1 = std::chrono::steady_clock::now();
        double us = std::chrono::duration<double, std::micro>(t1 - t0).count() / reps;
        std::cout << "  n = " << n << "\t: " << us << " us per copy  (" << us * 1000 / n
                  << " ns per medicine)" << (sink < 0 ? "!" : "") << '\n';
    }
    return 0;
}
