// Q1 - Constructors and Constructor Overloading : Library Book Records
// Compile: g++ -std=c++17 -Wall -Wextra book.cpp -o book
#include <climits>
#include <iostream>
#include <stdexcept>
#include <string>

class Book {
    std::string title;   // initialised 1st (declaration order)
    std::string author;  // initialised 2nd
    int copies;          // initialised 3rd

    // The ONLY place where the number of copies is checked.
    static int checkCopies(int c) {
        if (c < 0)
            throw std::invalid_argument("number of copies cannot be negative (got " +
                                        std::to_string(c) + ")");
        return c;
    }

public:
    // (1) Parameterised constructor - a new book with full details (target constructor)
    Book(const std::string& t, const std::string& a, int c)
        : title(t), author(a), copies(checkCopies(c)) {
        std::cout << "full-ctor ";
    }

    // (2) Overloaded constructor - a donated book with only a title (delegates to (1))
    explicit Book(const std::string& t) : Book(t, "Unknown (donated)", 1) {
        std::cout << "-> title-only-ctor ";
    }

    // (3) Default constructor - an empty record to be filled later (delegates to (1))
    Book() : Book("", "", 0) { std::cout << "-> default-ctor "; }

    // Filling an empty record later re-uses the same check, so the invariant
    // "copies >= 0" holds for the whole life of the object.
    void fill(const std::string& t, const std::string& a, int c) {
        copies = checkCopies(c);
        title = t;
        author = a;
    }

    void print() const {
        std::cout << "Book{ title=\"" << (title.empty() ? "<empty>" : title)
                  << "\", author=\"" << (author.empty() ? "<empty>" : author)
                  << "\", copies=" << copies << " }\n";
    }
};

int main() {
    std::cout << "--- 1. Creating Book objects with every constructor ---\n";
    std::cout << "b1: "; Book b1("The Pragmatic Programmer", "Hunt & Thomas", 5); std::cout << '\n';
    std::cout << "b2: "; Book b2("Wings of Fire");                                std::cout << '\n';
    std::cout << "b3: "; Book b3;                                                  std::cout << '\n';
    b1.print();
    b2.print();
    b3.print();

    std::cout << "\n--- 2. Array of Books (uses the default constructor 3 times) ---\n";
    Book shelf[3];
    std::cout << '\n';
    shelf[0].fill("Clean Code", "Robert C. Martin", 2);
    for (const Book& b : shelf) b.print();

    std::cout << "\n--- 3. Invalid number of copies ---\n";
    for (int bad : {-1, -100, INT_MIN /* simulates a garbage value */}) {
        try {
            Book damaged("Damaged Record", "X", bad);
            damaged.print();   // never reached
        } catch (const std::invalid_argument& e) {
            std::cout << "Rejected: " << e.what() << '\n';
        }
    }
    try {
        b3.fill("Late Entry", "Y", -5);
    } catch (const std::invalid_argument& e) {
        std::cout << "fill() rejected: " << e.what() << "  -> b3 unchanged: ";
        b3.print();
    }
    return 0;
}
