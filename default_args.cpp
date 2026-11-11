// Q1 alternative: ONE constructor with default arguments instead of overloads + delegation.
// Compile: g++ -std=c++17 -O2 -Wall -Wextra default_args.cpp -o default_args
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>

class BookDA {   // default-argument design
    std::string title, author;
    int copies;
    static int checkCopies(int c) {
        if (c < 0) throw std::invalid_argument("negative copies");
        return c;
    }
public:
    explicit BookDA(const std::string& t = "", const std::string& a = "Unknown (donated)", int c = 0)
        : title(t), author(a), copies(checkCopies(c)) {}   // check in ONE place
    int getCopies() const { return copies; }
};

class BookDel {  // overload + delegation design (same as book.cpp, without messages)
    std::string title, author;
    int copies;
    static int checkCopies(int c) {
        if (c < 0) throw std::invalid_argument("negative copies");
        return c;
    }
public:
    BookDel(const std::string& t, const std::string& a, int c)
        : title(t), author(a), copies(checkCopies(c)) {}   // check in ONE place
    explicit BookDel(const std::string& t) : BookDel(t, "Unknown (donated)", 1) {}
    BookDel() : BookDel("", "", 0) {}
    int getCopies() const { return copies; }
};

template <class B, class... Args>
double timeCreate(int n, Args... args) {
    long long sink = 0;
    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < n; ++i) { B b(args...); sink += b.getCopies(); }
    auto t1 = std::chrono::steady_clock::now();
    if (sink == -1) std::cout << "";
    return std::chrono::duration<double, std::nano>(t1 - t0).count() / n;
}

int main() {
    // Behavioural difference: with default args a title-only book gets copies = 0,
    // the overloaded design can give a donated book copies = 1.
    BookDA da("Wings of Fire");
    BookDel dl("Wings of Fire");
    std::cout << "Title-only book: default-args copies = " << da.getCopies()
              << ", delegation copies = " << dl.getCopies() << '\n';
    std::cout << "sizeof(BookDA) = " << sizeof(BookDA) << " bytes, sizeof(BookDel) = "
              << sizeof(BookDel) << " bytes\n";

    const int N = 1000000;
    std::cout << "Average construction time over " << N << " objects:\n";
    std::cout << "  default-args, 3 args : " << timeCreate<BookDA>(N, std::string("T"), std::string("A"), 3) << " ns\n";
    std::cout << "  delegation,   3 args : " << timeCreate<BookDel>(N, std::string("T"), std::string("A"), 3) << " ns\n";
    std::cout << "  default-args, 1 arg  : " << timeCreate<BookDA>(N, std::string("T")) << " ns\n";
    std::cout << "  delegation,   1 arg  : " << timeCreate<BookDel>(N, std::string("T")) << " ns\n";
}
