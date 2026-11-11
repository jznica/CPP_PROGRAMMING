// Q1 experiment: default constructor REMOVED -> an array of Books cannot be created.
// Compile: g++ -std=c++17 -Wall -Wextra no_default_ctor.cpp   (expected to FAIL)
#include <string>

class Book {
    std::string title;
    int copies;
public:
    Book(const std::string& t, int c) : title(t), copies(c) {}   // user-declared ctor
    // No Book() : the compiler does NOT supply one because another ctor is declared.
};

int main() {
    Book shelf[3];   // error: no matching function for call to 'Book::Book()'
    (void)shelf;
}
