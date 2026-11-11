// Q1 experiment: member initialised in the WRONG order -> -Wreorder warning.
// Compile: g++ -std=c++17 -Wall -Wextra reorder.cpp -o reorder
#include <iostream>

struct BookStock {
    int total;    // declared FIRST  -> initialised FIRST
    int copies;   // declared SECOND -> initialised SECOND
    // The list is written copies-then-total, but C++ ignores list order:
    // total(copies * 2) runs first and reads the still-uninitialised 'copies'.
    explicit BookStock(int c) : copies(c), total(copies * 2) {}
};

int main() {
    BookStock s(5);
    std::cout << "copies = " << s.copies << ", total = " << s.total
              << "   (expected total = 10)\n";
}
