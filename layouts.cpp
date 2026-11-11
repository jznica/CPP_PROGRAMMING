// Q3 alternative: row-pointer map vs one flat block vs std::vector<std::vector<char>>
// Compile: g++ -std=c++17 -Wall -Wextra -O2 layouts.cpp -o layouts && ./layouts
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <new>
#include <vector>

static long long g_allocs = 0, g_bytes = 0;
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

struct RowPtrMap {   // original design
    int R, S; char** a;
    RowPtrMap(int r, int s) : R(r), S(s), a(new char*[r]) { for (int i = 0; i < R; ++i) a[i] = new char[S](); }
    ~RowPtrMap() { for (int i = 0; i < R; ++i) delete[] a[i]; delete[] a; }
    RowPtrMap(const RowPtrMap&) = delete; RowPtrMap& operator=(const RowPtrMap&) = delete;
    char& at(int r, int s) { return a[r][s]; }
};
struct FlatMap {     // one block, index r * S + s
    int R, S; char* a;
    FlatMap(int r, int s) : R(r), S(s), a(new char[static_cast<std::size_t>(r) * s]()) {}
    ~FlatMap() { delete[] a; }
    FlatMap(const FlatMap&) = delete; FlatMap& operator=(const FlatMap&) = delete;
    char& at(int r, int s) { return a[static_cast<std::size_t>(r) * S + s]; }
};
struct VecMap {      // Rule of Zero
    std::vector<std::vector<char>> a;
    VecMap(int r, int s) : a(r, std::vector<char>(s, 0)) {}
    char& at(int r, int s) { return a[r][s]; }
};

template <class M>
void measure(const char* name, int R, int S) {
    long long a0 = g_allocs, b0 = g_bytes;
    M m(R, S);
    long long allocs = g_allocs - a0, bytes = g_bytes - b0;
    // access time: touch every seat 20 times
    auto t0 = std::chrono::steady_clock::now();
    long long sum = 0;
    for (int k = 0; k < 20; ++k)
        for (int r = 0; r < R; ++r)
            for (int s = 0; s < S; ++s) { m.at(r, s) += 1; sum += m.at(r, s); }
    auto t1 = std::chrono::steady_clock::now();
    double ns = std::chrono::duration<double, std::nano>(t1 - t0).count() / (20.0 * R * S);
    std::cout << "  " << name << "\t allocations = " << allocs << "\t bytes requested = " << bytes
              << "\t access = " << ns << " ns/seat" << (sum == -1 ? "!" : "") << '\n';
}

int main() {
    for (auto rs : {std::pair<int, int>{10, 20}, {100, 200}, {1000, 2000}}) {
        std::cout << rs.first << " x " << rs.second << ":\n";
        measure<RowPtrMap>("row pointers", rs.first, rs.second);
        measure<FlatMap>("flat block  ", rs.first, rs.second);
        measure<VecMap>("vector<vector>", rs.first, rs.second);
    }
}
