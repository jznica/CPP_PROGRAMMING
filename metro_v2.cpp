// Task 12 - revised design (decision: MODIFY)
//   * fourth zone Airport (Rs 60) in a zone-indexed table sized by Zone::Count
//   * 10 % weekend discount that never combines with the peak surcharge
//   * monthly pass: unlimited journeys inside two chosen zones
// Compile: g++ -std=c++17 -Wall -Wextra metro_v2.cpp -o metro_v2
#include <algorithm>
#include <array>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

enum class Zone : int { Inner, Middle, Outer, Airport, Count };
enum class Passenger { Regular, Student, Senior };
enum class DayType { Weekday, Weekend };

constexpr std::size_t ZONE_COUNT = static_cast<std::size_t>(Zone::Count);
constexpr std::array<long long, ZONE_COUNT> BASE_PAISE{1000, 2000, 3000, 6000};
static_assert(BASE_PAISE.size() == ZONE_COUNT, "one fare per zone");

constexpr int PEAK_SURCHARGE_PERCENT = 20;
constexpr int WEEKEND_DISCOUNT_PERCENT = 10;
constexpr int STUDENT_CONCESSION_PERCENT = 25;
constexpr int SENIOR_CONCESSION_PERCENT = 50;
constexpr long long MIN_BALANCE_PAISE = 5000;

bool isPeak(int minuteOfDay)                       // unchanged from v1: [08:00,10:00) and [17:00,20:00)
{
    return (minuteOfDay >= 8 * 60 && minuteOfDay < 10 * 60) ||
           (minuteOfDay >= 17 * 60 && minuteOfDay < 20 * 60);
}

struct Journey {
    Zone from, to;
    int minuteOfDay;
    DayType day;
    Passenger passenger;
    int yyyymm;                                    // travel month, e.g. 202611
    Zone fareZone() const { return std::max(from, to); }   // farthest zone travelled
};

long long baseFare(Zone z) { return BASE_PAISE.at(static_cast<std::size_t>(z)); }   // bounds-checked

int concessionPercent(Passenger p)
{
    switch (p) {
    case Passenger::Regular: return 0;
    case Passenger::Student: return STUDENT_CONCESSION_PERCENT;
    case Passenger::Senior:  return SENIOR_CONCESSION_PERCENT;
    }
    return 0;
}

// Weekend discount and peak surcharge never combine: weekend wins, peak ignored.
int timeAdjustmentPercent(int minuteOfDay, DayType day)
{
    if (day == DayType::Weekend) return -WEEKEND_DISCOUNT_PERCENT;
    return isPeak(minuteOfDay) ? PEAK_SURCHARGE_PERCENT : 0;
}

long long fare(const Journey& j)
{
    const long long adj = timeAdjustmentPercent(j.minuteOfDay, j.day);
    const long long con = concessionPercent(j.passenger);
    return (baseFare(j.fareZone()) * (100 + adj) * (100 - con) + 5000) / 10000;
}

class MonthlyPass {
public:
    MonthlyPass(Zone a, Zone b, int yyyymm) : yyyymm_(yyyymm)
    {
        if (a == b) throw std::invalid_argument("a pass needs two different zones");
        mask_ = bit(a) | bit(b);
    }
    // Covered only if every zone the journey passes through is one of the two chosen zones.
    bool covers(const Journey& j) const
    {
        if (j.yyyymm != yyyymm_) return false;
        int lo = static_cast<int>(std::min(j.from, j.to)), hi = static_cast<int>(std::max(j.from, j.to));
        for (int z = lo; z <= hi; ++z)
            if (!(mask_ & bit(static_cast<Zone>(z)))) return false;
        return true;
    }
private:
    static unsigned bit(Zone z) { return 1u << static_cast<unsigned>(z); }
    unsigned mask_ = 0;
    int yyyymm_;
};

class SmartCard {
public:
    explicit SmartCard(long long openingPaise)
    {
        if (openingPaise < 0) throw std::invalid_argument("opening balance < 0");
        balance_ = openingPaise;
    }
    long long balance() const { return balance_; }
    void attachPass(const MonthlyPass& p) { pass_ = p; }
    long long chargeFor(const Journey& j) const { return pass_ && pass_->covers(j) ? 0 : fare(j); }
    bool canTravel(long long f) const { return f >= 0 && (f == 0 || balance_ - f >= MIN_BALANCE_PAISE); }
    bool travel(const Journey& j)
    {
        long long f = chargeFor(j);
        if (!canTravel(f)) return false;
        balance_ -= f;
        return true;
    }
    bool recharge(long long paise)
    {
        if (paise <= 0) return false;
        balance_ += paise;
        return true;
    }
private:
    long long balance_ = 0;                        // invariant: never negative
    std::optional<MonthlyPass> pass_;
};

// ---------------- tests ----------------
int passed = 0, failed = 0;

void check(const std::string& id, const std::string& what, long long expected, long long actual)
{
    bool ok = expected == actual;
    (ok ? passed : failed)++;
    std::cout << std::left << std::setw(5) << id << std::setw(49) << what << std::right
              << std::setw(9) << expected << std::setw(8) << actual << (ok ? "  PASS" : "  FAIL") << '\n';
}

constexpr int HM(int h, int m) { return h * 60 + m; }
constexpr int NOV = 202611, DEC = 202612;

Journey trip(Zone z, int minute, DayType d, Passenger p, int month = NOV) { return {z, z, minute, d, p, month}; }

int main()
{
    using P = Passenger;
    const DayType WD = DayType::Weekday, WE = DayType::Weekend;
    std::cout << std::left << std::setw(5) << "ID" << std::setw(49) << "Case" << std::right
              << std::setw(9) << "Expected" << std::setw(8) << "Actual" << "  Result\n";

    // R: v1 weekday cases must not change (regression)
    check("R1", "Inner 12:00 regular", 1000, fare(trip(Zone::Inner, HM(12, 0), WD, P::Regular)));
    check("R2", "Outer 09:15 regular peak", 3600, fare(trip(Zone::Outer, HM(9, 15), WD, P::Regular)));
    check("R3", "Middle 10:00 regular (peak over)", 2000, fare(trip(Zone::Middle, HM(10, 0), WD, P::Regular)));
    check("R4", "Middle 19:59 regular (peak)", 2400, fare(trip(Zone::Middle, HM(19, 59), WD, P::Regular)));
    check("R5", "Middle 20:00 regular (peak over)", 2000, fare(trip(Zone::Middle, HM(20, 0), WD, P::Regular)));
    check("R6", "Outer 08:30 student peak", 2700, fare(trip(Zone::Outer, HM(8, 30), WD, P::Student)));
    check("R7", "Middle 18:30 senior peak", 1200, fare(trip(Zone::Middle, HM(18, 30), WD, P::Senior)));

    // A: the new Airport zone
    check("A1", "Airport 12:00 weekday regular", 6000, fare(trip(Zone::Airport, HM(12, 0), WD, P::Regular)));
    check("A2", "Airport 08:30 weekday regular (peak)", 7200, fare(trip(Zone::Airport, HM(8, 30), WD, P::Regular)));
    check("A3", "Airport 12:00 weekend regular", 5400, fare(trip(Zone::Airport, HM(12, 0), WE, P::Regular)));
    check("A4", "Airport 18:00 weekday senior", 3600, fare(trip(Zone::Airport, HM(18, 0), WD, P::Senior)));

    // W: weekend discount never combines with the peak surcharge
    check("W1", "Inner 08:30 weekend (discount, no surcharge)", 900, fare(trip(Zone::Inner, HM(8, 30), WE, P::Regular)));
    check("W2", "Outer 18:00 weekend student", 2025, fare(trip(Zone::Outer, HM(18, 0), WE, P::Student)));
    check("W3", "Middle 12:00 weekend senior", 900, fare(trip(Zone::Middle, HM(12, 0), WE, P::Senior)));
    check("W4", "Inner 12:00 weekend student", 675, fare(trip(Zone::Inner, HM(12, 0), WE, P::Student)));

    // P: monthly pass for Inner + Middle in November
    SmartCard card(5000);                          // exactly Rs 50 on the card
    card.attachPass(MonthlyPass(Zone::Inner, Zone::Middle, NOV));
    Journey innerToMiddlePeak{Zone::Inner, Zone::Middle, HM(8, 30), WD, P::Regular, NOV};
    Journey middleToMiddle{Zone::Middle, Zone::Middle, HM(12, 0), WD, P::Regular, NOV};
    Journey innerToOuter{Zone::Inner, Zone::Outer, HM(12, 0), WD, P::Regular, NOV};
    Journey december{Zone::Inner, Zone::Middle, HM(12, 0), WD, P::Regular, DEC};
    check("P1", "pass Inner->Middle peak: charge", 0, card.chargeFor(innerToMiddlePeak));
    check("P2", "pass Middle->Middle: charge", 0, card.chargeFor(middleToMiddle));
    check("P3", "pass, card at Rs 50: covered trip allowed", 1, card.travel(innerToMiddlePeak));
    check("P4", "pass Inner->Outer (Outer not in pass)", 3000, card.chargeFor(innerToOuter));
    check("P5", "  ...denied at Rs 50 balance", 0, card.travel(innerToOuter));
    check("P6", "pass used in December (expired)", 2000, card.chargeFor(december));
    card.travel(december);                          // denied as well: Rs 50 floor

    bool sameZoneRejected = false;
    try { MonthlyPass bad(Zone::Outer, Zone::Outer, NOV); } catch (const std::invalid_argument&) { sameZoneRejected = true; }
    check("P7", "pass with the same zone twice rejected", 1, sameZoneRejected);

    bool unknownZoneRejected = false;
    try { baseFare(static_cast<Zone>(7)); } catch (const std::out_of_range&) { unknownZoneRejected = true; }
    check("P8", "unknown zone 7 rejected by table .at()", 1, unknownZoneRejected);
    check("P9", "balance never below 0 after all trips", 5000, card.balance());

    std::cout << '\n' << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
