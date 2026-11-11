// Task 9 - complete fare logic and its tests
// Compile: g++ -std=c++17 -Wall -Wextra metro_fare.cpp -o metro_fare
#include <array>
#include <cctype>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

// ---------------- constants: the only place fares and limits are written ----------------
enum class Zone { Inner, Middle, Outer };
enum class Passenger { Regular, Student, Senior };

constexpr std::array<long long, 3> BASE_PAISE{1000, 2000, 3000};
constexpr int PEAK_SURCHARGE_PERCENT = 20;
constexpr int STUDENT_CONCESSION_PERCENT = 25;      // assumption: rate not given in the brief
constexpr int SENIOR_CONCESSION_PERCENT = 50;       // assumption: rate not given in the brief
constexpr long long MIN_BALANCE_PAISE = 5000;
constexpr int MORNING_START = 8 * 60, MORNING_END = 10 * 60;   // [08:00, 10:00)
constexpr int EVENING_START = 17 * 60, EVENING_END = 20 * 60;  // [17:00, 20:00)

static_assert(BASE_PAISE[0] < BASE_PAISE[1] && BASE_PAISE[1] < BASE_PAISE[2],
              "fares must increase outward");

// ---------------- fare rules ----------------
bool isPeak(int minuteOfDay)
{
    return (minuteOfDay >= MORNING_START && minuteOfDay < MORNING_END) ||
           (minuteOfDay >= EVENING_START && minuteOfDay < EVENING_END);
}

int concessionPercent(Passenger p)
{
    switch (p) {
    case Passenger::Regular: return 0;
    case Passenger::Student: return STUDENT_CONCESSION_PERCENT;
    case Passenger::Senior:  return SENIOR_CONCESSION_PERCENT;
    }
    return 0;
}

long long baseFare(Zone z) { return BASE_PAISE[static_cast<std::size_t>(z)]; }

// One rounding step, half-up to the nearest paisa: base * (100+s)/100 * (100-c)/100
long long fare(Zone z, int minuteOfDay, Passenger p)
{
    const long long surcharge = isPeak(minuteOfDay) ? PEAK_SURCHARGE_PERCENT : 0;
    const long long concession = concessionPercent(p);
    return (baseFare(z) * (100 + surcharge) * (100 - concession) + 5000) / 10000;
}

// ---------------- input validation: reject, never guess ----------------
std::optional<Zone> zoneFromNumber(int n)          // keypad: 1 = Inner, 2 = Middle, 3 = Outer
{
    if (n < 1 || n > 3) return std::nullopt;
    return static_cast<Zone>(n - 1);
}

std::optional<int> parseTime(const std::string& hhmm)   // "HH:MM" -> minute of day
{
    if (hhmm.size() != 5 || hhmm[2] != ':') return std::nullopt;
    for (int i : {0, 1, 3, 4})
        if (!std::isdigit(static_cast<unsigned char>(hhmm[i]))) return std::nullopt;
    int h = std::stoi(hhmm.substr(0, 2)), m = std::stoi(hhmm.substr(3, 2));
    if (h > 23 || m > 59) return std::nullopt;
    return h * 60 + m;
}

// ---------------- the card ----------------
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
        return farePaise >= 0 && balance_ - farePaise >= MIN_BALANCE_PAISE;
    }
    bool deduct(long long farePaise)
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
    long long balance_ = 0;
};

// ---------------- tests ----------------
int passed = 0, failed = 0;

void check(const std::string& id, const std::string& what, long long expected, long long actual)
{
    bool ok = expected == actual;
    (ok ? passed : failed)++;
    std::cout << std::left << std::setw(5) << id << std::setw(47) << what << std::right
              << std::setw(10) << expected << std::setw(9) << actual << (ok ? "  PASS" : "  FAIL")
              << '\n';
}

struct FareCase {
    const char* id;
    const char* what;
    Zone zone;
    const char* time;
    Passenger passenger;
    long long expected;
};

const FareCase FARE_CASES[] = {
    {"N1",  "Inner  12:00 regular (off-peak)", Zone::Inner,  "12:00", Passenger::Regular, 1000},
    {"N2",  "Middle 12:00 regular (off-peak)", Zone::Middle, "12:00", Passenger::Regular, 2000},
    {"N3",  "Outer  12:00 regular (off-peak)", Zone::Outer,  "12:00", Passenger::Regular, 3000},
    {"N4",  "Inner  08:30 regular (peak)",     Zone::Inner,  "08:30", Passenger::Regular, 1200},
    {"N5",  "Middle 18:30 regular (peak)",     Zone::Middle, "18:30", Passenger::Regular, 2400},
    {"N6",  "Outer  09:15 regular (peak)",     Zone::Outer,  "09:15", Passenger::Regular, 3600},
    {"N7",  "Inner  12:00 student",            Zone::Inner,  "12:00", Passenger::Student, 750},
    {"N8",  "Outer  08:30 student (peak)",     Zone::Outer,  "08:30", Passenger::Student, 2700},
    {"N9",  "Middle 18:30 senior (peak)",      Zone::Middle, "18:30", Passenger::Senior,  1200},
    {"N10", "Outer  12:00 senior",             Zone::Outer,  "12:00", Passenger::Senior,  1500},
    {"B1",  "Middle 07:59 (just before peak)", Zone::Middle, "07:59", Passenger::Regular, 2000},
    {"B2",  "Middle 08:00 (peak starts)",      Zone::Middle, "08:00", Passenger::Regular, 2400},
    {"B3",  "Middle 09:59 (last peak minute)", Zone::Middle, "09:59", Passenger::Regular, 2400},
    {"B4",  "Middle 10:00 (peak over)",        Zone::Middle, "10:00", Passenger::Regular, 2000},
    {"B5",  "Middle 17:00 (peak starts)",      Zone::Middle, "17:00", Passenger::Regular, 2400},
    {"B6",  "Middle 19:59 (last peak minute)", Zone::Middle, "19:59", Passenger::Regular, 2400},
    {"B7",  "Middle 20:00 (peak over)",        Zone::Middle, "20:00", Passenger::Regular, 2000},
    {"B8",  "Inner  00:00 (midnight)",         Zone::Inner,  "00:00", Passenger::Regular, 1000},
};

int main()
{
    std::cout << std::left << std::setw(5) << "ID" << std::setw(47) << "Case" << std::right
              << std::setw(10) << "Expected" << std::setw(9) << "Actual" << "  Result\n";

    // normal and boundary fares: one table, one loop
    for (const FareCase& c : FARE_CASES)
        check(c.id, c.what, c.expected, fare(c.zone, *parseTime(c.time), c.passenger));

    // balance boundary: exactly Rs 50 left is allowed
    SmartCard edge(6000);
    check("B9", "Rs 60 card, Rs 10 fare -> leaves Rs 50 (allow)", 1, edge.deduct(1000));
    check("B10", "  ...balance afterwards", 5000, edge.balance());
    SmartCard below(5999);
    check("B11", "Rs 59.99 card, Rs 10 fare (deny)", 0, below.deduct(1000));
    check("B12", "  ...balance unchanged", 5999, below.balance());

    // invalid cases
    check("I1", "zone number 4 rejected", 0, zoneFromNumber(4).has_value());
    check("I2", "zone number 0 rejected", 0, zoneFromNumber(0).has_value());
    SmartCard card(10000);
    check("I3", "recharge of -Rs 5 rejected", 0, card.recharge(-500));
    check("I4", "  ...balance unchanged", 10000, card.balance());
    check("I5", "hour typed as letters \"ab:cd\" rejected", 0, parseTime("ab:cd").has_value());
    check("I6", "time \"24:00\" rejected", 0, parseTime("24:00").has_value());
    check("I7", "negative fare cannot be deducted", 0, card.deduct(-100));
    bool threw = false;
    try { SmartCard bad(-1); } catch (const std::invalid_argument&) { threw = true; }
    check("I8", "negative opening balance throws", 1, threw);

    // five journeys calculated by hand (card starts at Rs 200.00)
    struct Trip { Zone z; const char* time; Passenger p; };
    const Trip day[] = {{Zone::Inner, "08:15", Passenger::Regular},
                        {Zone::Outer, "12:00", Passenger::Student},
                        {Zone::Middle, "18:00", Passenger::Senior},
                        {Zone::Outer, "20:00", Passenger::Regular},
                        {Zone::Middle, "09:59", Passenger::Student}};
    SmartCard daily(20000);
    long long total = 0;
    for (const Trip& t : day) {
        long long f = fare(t.z, *parseTime(t.time), t.p);
        if (daily.deduct(f)) total += f;
    }
    check("H1", "5-journey total (hand: Rs 94.50)", 9450, total);
    check("H2", "5-journey closing balance (hand: Rs 105.50)", 10550, daily.balance());

    std::cout << '\n' << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
