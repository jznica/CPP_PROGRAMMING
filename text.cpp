// Q6 - The Rules of Operator Overloading : Text class for an SMS gateway
// Compile: g++ -std=c++17 -Wall -Wextra -g -fsanitize=address text.cpp -o text_asan && ./text_asan
//          g++ -std=c++17 -Wall -Wextra -O2 text.cpp -o text && ./text       (timings)
//          g++ -std=c++17 -Wall -Wextra -fno-elide-constructors text.cpp -o text_noelide
#include <chrono>
#include <cstring>
#include <iostream>
#include <string>

class Text {
    std::size_t len;
    char* data;                       // dynamically allocated, '\0'-terminated

    struct Uninit {};
    Text(std::size_t n, Uninit) : len(n), data(new char[n + 1]) { ++allocations; }

public:
    static long copies, allocations, charsCopied;

    Text(const char* s = "") : len(std::strlen(s)), data(new char[len + 1]) {
        std::memcpy(data, s, len + 1);
        ++allocations;
    }
    Text(const Text& o) : len(o.len), data(new char[o.len + 1]) {        // deep copy
        std::memcpy(data, o.data, len + 1);
        ++copies; ++allocations; charsCopied += len;
    }
    Text& operator=(const Text& o) {                                      // must be a member
        if (this == &o) return *this;                                     // self-assignment guard
        char* fresh = new char[o.len + 1];                                // allocate first
        std::memcpy(fresh, o.data, o.len + 1);
        delete[] data;
        data = fresh;
        len = o.len;
        ++allocations; charsCopied += len;
        return *this;
    }
    ~Text() { delete[] data; }

    Text& operator+=(const Text& o) {
        char* fresh = new char[len + o.len + 1];
        std::memcpy(fresh, data, len);
        std::memcpy(fresh + len, o.data, o.len + 1);   // also safe when &o == this
        delete[] data;
        data = fresh;
        charsCopied += len + o.len;
        len += o.len;
        ++allocations;
        return *this;
    }

    // Binary operators as non-member friends: symmetric, so "Hi " + t also works
    friend Text operator+(const Text& a, const Text& b) {
        Text r(a.len + b.len, Uninit{});
        std::memcpy(r.data, a.data, a.len);
        std::memcpy(r.data + a.len, b.data, b.len + 1);
        charsCopied += a.len + b.len;
        return r;
    }
    friend bool operator==(const Text& a, const Text& b) {
        return a.len == b.len && std::memcmp(a.data, b.data, a.len) == 0;
    }
    friend bool operator!=(const Text& a, const Text& b) { return !(a == b); }
    friend std::ostream& operator<<(std::ostream& os, const Text& t) { return os << t.data; }

    std::size_t size() const { return len; }
    int smsParts() const { return len == 0 ? 1 : len <= 160 ? 1 : static_cast<int>((len + 152) / 153); }
};
long Text::copies = 0, Text::allocations = 0, Text::charsCopied = 0;

static void resetCounters() { Text::copies = Text::allocations = Text::charsCopied = 0; }

int main() {
    std::cout << "--- 1. Basic operations ---\n";
    Text greet("Your OTP is "), code("482913"), dot(".");
    Text msg = greet + code + dot;
    std::cout << "  msg            : " << msg << "  (" << msg.size() << " chars)\n";
    Text copy = msg;
    std::cout << std::boolalpha << "  copy == msg    : " << (copy == msg) << '\n';
    copy += Text(" Do not share.");
    std::cout << "  copy != msg    : " << (copy != msg) << "  -> " << copy << '\n';
    std::cout << "  \"Hi \" + code    : " << "Hi " + code << "   (works because + is a non-member)\n";

    std::cout << "\n--- 2. Self-assignment ---\n";
    Text& alias = msg;
    msg = alias;
    std::cout << "  after msg = msg : " << msg << '\n';
    msg += msg;
    std::cout << "  after msg += msg: " << msg << '\n';

    std::cout << "\n--- 3. Empty and long messages ---\n";
    Text empty, empty2("");
    std::cout << "  empty: size " << empty.size() << ", empty == \"\" : " << (empty == empty2)
              << ", empty + code = " << empty + code << '\n';
    Text longMsg;
    for (int i = 0; i < 50; ++i) longMsg += Text("SMS-text ");
    std::cout << "  long : size " << longMsg.size() << " chars -> " << longMsg.smsParts()
              << " SMS parts (153 chars each after the first 160)\n";

    std::cout << "\n--- 4. Copies made by a + b + c (each operand 1000 chars) ---\n";
    std::string s1000(1000, 'a');
    Text a(s1000.c_str()), b(s1000.c_str()), c(s1000.c_str());
    resetCounters();
    Text abc = a + b + c;
    std::cout << "  copy-constructor calls = " << Text::copies << ", allocations = "
              << Text::allocations << ", characters copied = " << Text::charsCopied << '\n';

    std::cout << "\n--- 5. 100,000 concatenations of one character ---\n";
    const int N = 100000;
    auto t0 = std::chrono::steady_clock::now();
    Text t; Text x("x");
    for (int i = 0; i < N; ++i) t = t + x;
    auto t1 = std::chrono::steady_clock::now();
    Text u;
    for (int i = 0; i < N; ++i) u += x;
    auto t2 = std::chrono::steady_clock::now();
    std::string s;
    for (int i = 0; i < N; ++i) s = s + "x";
    auto t3 = std::chrono::steady_clock::now();
    std::string v;
    for (int i = 0; i < N; ++i) v += "x";
    auto t4 = std::chrono::steady_clock::now();
    auto ms = [](auto p, auto q) { return std::chrono::duration<double, std::milli>(q - p).count(); };
    std::cout << "  Text        t = t + x : " << ms(t0, t1) << " ms\n"
              << "  Text        t += x    : " << ms(t1, t2) << " ms\n"
              << "  std::string s = s + x : " << ms(t2, t3) << " ms\n"
              << "  std::string s += x    : " << ms(t3, t4) << " ms\n"
              << "  lengths: " << t.size() << ' ' << u.size() << ' ' << s.size() << ' ' << v.size() << '\n';
}
