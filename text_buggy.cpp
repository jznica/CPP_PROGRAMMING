// Q6 - FIRST DRAFT of Text (kept to show what the address sanitizer found).
// Bug 1: constructor allocates len bytes instead of len + 1 (no room for '\0').
// Bug 2: operator= deletes its own buffer before copying (breaks t = t).
// Compile: g++ -std=c++17 -Wall -Wextra -g -fsanitize=address text_buggy.cpp -o text_buggy
// Run    : ./text_buggy overflow      ./text_buggy self
#include <cstring>
#include <iostream>

class Text {
    std::size_t len;
    char* data;
public:
    Text(const char* s, bool buggy) : len(std::strlen(s)), data(new char[buggy ? len : len + 1]) {
        std::strcpy(data, s);                     // writes len + 1 bytes
    }
    Text(const Text& o) : len(o.len), data(new char[o.len + 1]) { std::strcpy(data, o.data); }
    Text& operator=(const Text& o) {              // no self-assignment guard
        delete[] data;                            // when &o == this, o.data is freed here ...
        len = o.len;
        data = new char[len + 1];
        std::strcpy(data, o.data);                // ... and read here
        return *this;
    }
    ~Text() { delete[] data; }
    friend std::ostream& operator<<(std::ostream& os, const Text& t) { return os << t.data; }
};

int main(int argc, char** argv) {
    if (argc > 1 && std::strcmp(argv[1], "overflow") == 0) {
        Text t("Hello", true);
        std::cout << t << '\n';
    } else {
        Text t("Hello", false);
        Text& same = t;
        t = same;                                 // self-assignment
        std::cout << t << '\n';
    }
}
