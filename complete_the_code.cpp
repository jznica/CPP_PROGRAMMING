// Q2 - Complete the Code (TODO 1 - TODO 3 completed)
// Compile: g++ -std=c++17 -Wall -Wextra -g -fsanitize=address complete_the_code.cpp -o ctc && ./ctc
#include <iostream>
#include <string>

class Prescription {
    int n;
    std::string* medicines; // dynamically allocated array
public:
    explicit Prescription(int count) : n(count), medicines(new std::string[count]) {}

    // TODO 1: Copy constructor: allocate a NEW array of the same size and copy every
    // medicine (deep copy)
    Prescription(const Prescription& other) : n(other.n), medicines(new std::string[other.n]) {
        for (int i = 0; i < n; ++i) medicines[i] = other.medicines[i];
    }

    // TODO 2: Copy assignment: guard against self-assignment, build a new array, copy,
    // release the old array and return *this
    Prescription& operator=(const Prescription& other) {
        if (this != &other) {
            std::string* fresh = new std::string[other.n];
            for (int i = 0; i < other.n; ++i) fresh[i] = other.medicines[i];
            delete[] medicines;
            medicines = fresh;
            n = other.n;
        }
        return *this;
    }

    ~Prescription()
    {
        // TODO 3: Release the dynamically allocated array
        delete[] medicines;
    }
    void set(int i, const std::string& m) { medicines[i] = m; }
    std::string get(int i) const { return medicines[i]; }
};

int main()
{
    Prescription p(2);
    p.set(0, "Paracetamol");
    p.set(1, "ORS");
    Prescription q = p; // copy constructor
    q.set(1, "Cetirizine");
    Prescription r(1);
    r = p; // copy assignment
    p.set(0, "Ibuprofen");
    std::cout << p.get(0) << ' ' << p.get(1) << " | " << q.get(0) << ' ' << q.get(1) << " | " << r.get(0) << ' ' << r.get(1) << '\n';
}
