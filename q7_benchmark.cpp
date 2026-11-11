// Q7 - Original (shifting array) vs Redesigned (circular buffer) mini-statement
// Same transactions, same history size K, same checks.
#include <chrono>
#include <cstring>
#include <iostream>
#include <iomanip>

struct Transaction { char type; double amount; double balanceAfter; };

// ---------- ORIGINAL: shift the array left when full (Q4 design) ----------
template <int K>
class ShiftAccount {
    double balance = 0;
    Transaction txn[K];
    int count = 0;
public:
    long long copies = 0;                       // instrumentation: element moves
    void deposit(double amt)
    {
        balance += amt;
        if (count == K) {
            for (int i = 1; i < K; ++i) txn[i - 1] = txn[i];
            copies += K - 1;
            --count;
        }
        txn[count++] = {'D', amt, balance};
    }
    const Transaction& at(int i) const { return txn[i]; }   // 0 = oldest
    int size() const { return count; }
};

// ---------- REDESIGNED: circular buffer with a start index ----------
template <int K>
class RingAccount {
    double balance = 0;
    Transaction txn[K];
    int start = 0;                              // index of oldest
    int count = 0;
public:
    long long copies = 0;
    void deposit(double amt)
    {
        balance += amt;
        int pos = start + count; if (pos >= K) pos -= K;    // avoid % in hot path
        txn[pos] = {'D', amt, balance};
        ++copies;                                         // exactly one write
        if (count < K) ++count;
        else if (++start == K) start = 0;                 // overwrite oldest
    }
    const Transaction& at(int i) const { int p = start + i; if (p >= K) p -= K; return txn[p]; }
    int size() const { return count; }
};

template <class Acc>
double nsPerTxn(Acc& acc, int n)
{
    auto t0 = std::chrono::steady_clock::now();
    for (int i = 1; i <= n; ++i) acc.deposit(i);
    auto t1 = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::nano>(t1 - t0).count() / n;
}

template <int K>
void compare(int n)
{
    static ShiftAccount<K> s;   // static: large K arrays off the stack
    static RingAccount<K>  r;
    double ts = 1e18, tr = 1e18;
    for (int rep = 0; rep < 3; ++rep) {          // best of 3 (fresh objects each rep)
        s = ShiftAccount<K>(); r = RingAccount<K>();
        double a = nsPerTxn(s, n), b = nsPerTxn(r, n);
        if (a < ts) ts = a;
        if (b < tr) tr = b;
    }
    // correctness: both must hold the same last K transactions in the same order
    bool same = s.size() == r.size();
    for (int i = 0; same && i < s.size(); ++i)
        same = s.at(i).amount == r.at(i).amount && s.at(i).balanceAfter == r.at(i).balanceAfter;
    std::cout << std::setw(6) << K << std::setw(12) << std::fixed << std::setprecision(2) << ts
              << std::setw(12) << tr << std::setw(10) << std::setprecision(1) << ts / tr << "x"
              << std::setw(14) << std::setprecision(2) << double(s.copies) / n << std::setw(10) << double(r.copies) / n
              << std::setw(8) << (same ? "yes" : "NO") << std::setw(10)
              << sizeof(ShiftAccount<K>) << std::setw(10) << sizeof(RingAccount<K>) << '\n';
}

int main()
{
    const int N = 1'000'000;     // transactions per run
    std::cout << "N = " << N << " deposits per run, best of 3\n";
    std::cout << "     K  shift ns/tx  ring ns/tx   speedup  moves/tx(sh)  (ring)  same?  bytes(sh) bytes(rg)\n";
    compare<10>(N);
    compare<50>(N);
    compare<200>(N);
    compare<1000>(N);
    compare<5000>(N / 10);
}
