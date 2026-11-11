// Task 7 - object-oriented version: SmartCard keeps its balance private
// Compile: g++ -std=c++17 -Wall -Wextra oop.cpp -o oop
// Error  : g++ -std=c++17 -Wall -Wextra -DSHOW_ERROR -c oop.cpp   (direct write is rejected)
#include <iostream>
#include <stdexcept>

enum class Zone { Inner, Middle, Outer };

struct FareTable {
    static constexpr long long BASE_PAISE[] = {1000, 2000, 3000};
    static constexpr int PEAK_SURCHARGE_PERCENT = 20;
    static constexpr long long MIN_BALANCE_PAISE = 5000;
    static long long fare(Zone z, bool peak)
    {
        long long base = BASE_PAISE[static_cast<int>(z)];
        return peak ? base * (100 + PEAK_SURCHARGE_PERCENT) / 100 : base;
    }
};

class SmartCard {
public:
    explicit SmartCard(long long openingPaise)
    {
        if (openingPaise < 0) throw std::invalid_argument("opening balance < 0");
        balance_ = openingPaise;
    }
    long long balance() const { return balance_; }
    bool canTravel(long long farePaise) const
    {
        return farePaise >= 0 && balance_ - farePaise >= FareTable::MIN_BALANCE_PAISE;
    }
    bool deduct(long long farePaise)             // the ONLY way money leaves the card
    {
        if (!canTravel(farePaise)) return false;
        balance_ -= farePaise;
        return true;
    }
    bool recharge(long long paise)
    {
        if (paise <= 0) return false;
        balance_ += paise;
        return true;
    }
private:
    long long balance_ = 0;                       // invariant: balance_ >= 0 always
};

void travel(SmartCard& card, Zone z, bool peak)
{
    long long f = FareTable::fare(z, peak);
    std::cout << (card.deduct(f) ? "travel  fare " : "DENIED  fare ") << f
              << "  balance " << card.balance() << '\n';
}

int main()
{
    SmartCard card(10000);
    travel(card, Zone::Inner, false);
    travel(card, Zone::Outer, true);
    travel(card, Zone::Middle, false);            // would leave Rs 34 -> denied
    std::cout << "deduct(6000) as emergency charge -> "
              << (card.deduct(6000) ? "accepted" : "rejected") << ", balance " << card.balance() << '\n';
#ifdef SHOW_ERROR
    // any other code trying to change the balance directly:
    card.balance_ -= 6000;                        // error: balance_ is private
#endif
    std::cout << "recharge(-500) -> " << (card.recharge(-500) ? "accepted" : "rejected") << '\n';
}
