// Q2 - version 1: NO copy constructor / copy assignment -> compiler-generated SHALLOW copy.
// Compile: g++ -std=c++17 -Wall -Wextra -g shallow.cpp -o shallow
//          g++ -std=c++17 -Wall -Wextra -g -fsanitize=address shallow.cpp -o shallow_asan
#include <iostream>
#include <string>

class Prescription {
    int n;
    std::string* medicines;   // dynamically allocated array
public:
    explicit Prescription(int count) : n(count), medicines(new std::string[count]) {}
    ~Prescription() { delete[] medicines; }   // destructor, but no copy ctor / operator=
    void set(int i, const std::string& m) { medicines[i] = m; }
    std::string get(int i) const { return medicines[i]; }
    const void* buffer() const { return medicines; }
    int size() const { return n; }
};

void show(const char* label, const Prescription& p) {
    std::cout << label << " [array at " << p.buffer() << "] : ";
    for (int i = 0; i < p.size(); ++i) std::cout << p.get(i) << (i + 1 < p.size() ? ", " : "\n");
}

int main() {
    Prescription original(2);
    original.set(0, "Paracetamol");
    original.set(1, "ORS");

    Prescription followUp = original;     // compiler-generated copy: copies the POINTER only
    followUp.set(1, "Cetirizine");        // doctor edits the follow-up prescription ...

    show("original ", original);          // ... and the original has changed as well
    show("follow-up", followUp);
    std::cout << "Leaving main: followUp's destructor deletes the array, then original's "
                 "destructor deletes the SAME array again (double delete)\n";
}
