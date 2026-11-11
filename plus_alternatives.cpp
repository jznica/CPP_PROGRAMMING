// Q6 alternative: three ways to evaluate a + b + c and how many characters each copies.
//  (A) friend operator+(const Text&, const Text&) building the result directly
//  (B) Text operator+(Text a, const Text& b) { a += b; return a; }
//  (C) std::string
// Compile: g++ -std=c++17 -Wall -Wextra -O2 plus_alternatives.cpp -o plus_alt && ./plus_alt
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <new>
#include <string>

static long g_allocs = 0, g_bytes = 0;
static void* countedAlloc(std::size_t n) {
    ++g_allocs; g_bytes += n;
    if (void* p = std::malloc(n)) return p;
    throw std::bad_alloc();
}
void* operator new(std::size_t n) { return countedAlloc(n); }
void* operator new[](std::size_t n) { return countedAlloc(n); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }
void operator delete[](void* p, std::size_t) noexcept { std::free(p); }

template <int Version>
class Text {
    std::size_t len; char* data;
public:
    static long copies, chars;
    Text(const char* s = "") : len(std::strlen(s)), data(new char[len + 1]) { std::memcpy(data, s, len +
     1); }
    Text(std::size_t n, char ch) : len(n), data(new char[n + 1]) { std::memset(data, ch, n); data[n] = 0; }
    Text(const Text& o) : len(o.len), data(new char[o.len + 1]) { std::memcpy(data, o.data, len + 1); ++copies; chars += len; }
    Text& operator=(const Text& o) {
        if (this != &o) { char* f = new char[o.len + 1]; std::memcpy(f, o.data, o.len + 1); delete[] data; data = f; len = o.len; chars += len; }
        return *this;
    }
    ~Text() { delete[] data; }
    Text& operator+=(const Text& o) {
        char* f = new char[len + o.len + 1];
        std::memcpy(f, data, len); std::memcpy(f + len, o.data, o.len + 1);
        delete[] data; data = f; chars += len + o.len; len += o.len;
        return *this;
    }
    std::size_t size() const { return len; }
    struct Raw {};
    Text(std::size_t n, Raw) : len(n), data(new char[n + 1]) {}
    char* buf() { return data; }
    const char* buf() const { return data; }
};
template <int V> long Text<V>::copies = 0;
template <int V> long Text<V>::chars = 0;

using TextA = Text<0>;
using TextB = Text<1>;

TextA operator+(const TextA& a, const TextA& b) {              // (A)
    TextA r(a.size() + b.size(), TextA::Raw{});
    std::memcpy(r.buf(), a.buf(), a.size());
    std::memcpy(r.buf() + a.size(), b.buf(), b.size() + 1);
    TextA::chars += a.size() + b.size();
    return r;
}
TextB operator+(TextB a, const TextB& b) { a += b; return a; }  // (B)

int main() {
    const std::size_t n1 = 1000, n2 = 2000, n3 = 3000;
    {
        TextA a(n1, 'a'), b(n2, 'b'), c(n3, 'c');
        long al = g_allocs, by = g_bytes;
        TextA r = a + b + c;
        std::cout << "(A) friend operator+ building result : copy-ctor calls = " << TextA::copies
                  << ", chars copied = " << TextA::chars << ", allocations = " << g_allocs - al
                  << ", bytes = " << g_bytes - by << "  (len " << r.size() << ")\n";
    }
    {
        TextB a(n1, 'a'), b(n2, 'b'), c(n3, 'c');
        long al = g_allocs, by = g_bytes;
        TextB r = a + b + c;
        std::cout << "(B) operator+(Text a, ...) { a += b }  : copy-ctor calls = " << TextB::copies
                  << ", chars copied = " << TextB::chars << ", allocations = " << g_allocs - al
                  << ", bytes = " << g_bytes - by << "  (len " << r.size() << ")\n";
    }
    {
        std::string a(n1, 'a'), b(n2, 'b'), c(n3, 'c');
        long al = g_allocs, by = g_bytes;
        std::string r = a + b + c;
        std::cout << "(C) std::string a + b + c              : allocations = " << g_allocs - al
                  << ", bytes = " << g_bytes - by << "  (len " << r.size() << ")\n";
    }
}
