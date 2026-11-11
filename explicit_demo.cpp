// Q1 design question: when should a constructor be explicit?
// Without explicit : g++ -std=c++17 -Wall -Wextra explicit_demo.cpp                (compiles)
// With explicit    : g++ -std=c++17 -Wall -Wextra -DUSE_EXPLICIT explicit_demo.cpp  (fails)
#include <iostream>
#include <string>

#ifdef USE_EXPLICIT
#define EXPLICIT explicit
#else
#define EXPLICIT
#endif

class Book {
    std::string title;
    int copies;
public:
    EXPLICIT Book(const std::string& t) : title(t), copies(1) {}
    void print() const { std::cout << "Book{" << title << ", " << copies << "}\n"; }
};

void shelve(const Book& b) { b.print(); }

int main() {
    std::string t = "Dune";
    shelve(t);        // implicit conversion std::string -> Book (a temporary book is created!)
    Book b = t;       // copy-initialisation also needs the implicit conversion
    b.print();
}
