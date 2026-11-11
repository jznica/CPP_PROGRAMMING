// Task 8 - safe input at the gate and a formatted journey report
// Compile: g++ -std=c++17 -Wall -Wextra gate_io.cpp -o gate_io
// Run    : ./gate_io < input_bad.txt
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

const char* ZONE_NAME[] = {"Inner", "Middle", "Outer"};
const long long BASE_PAISE[] = {1000, 2000, 3000};

bool isPeak(int minute)
{
    return (minute >= 8 * 60 && minute < 10 * 60) || (minute >= 17 * 60 && minute < 20 * 60);
}

void discardLine() { std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); }

// Reads an int in [lo, hi]; on letters or out-of-range values, reports and asks again.
bool readInt(const std::string& prompt, int lo, int hi, int& out)
{
    while (true) {
        std::cout << prompt;
        if (std::cin >> out) {
            discardLine();                           // drop trailing junk such as "2x"
            if (out >= lo && out <= hi) return true;
            std::cout << "  [range error] enter " << lo << ".." << hi << '\n';
            continue;
        }
        if (std::cin.eof()) return false;           // no more input
        std::cin.clear();                            // clear failbit
        discardLine();                               // discard the bad token
        std::cout << "  [input error] numbers only\n";
    }
}

std::string money(long long paise)
{
    std::string p = std::to_string(paise % 100);
    return "Rs " + std::to_string(paise / 100) + '.' + (p.size() < 2 ? "0" + p : p);
}

struct Journey { int zone, hour, minute; long long fare; bool peak; };

int main()
{
    int n = 0;
    if (!readInt("Number of journeys: ", 1, 100, n)) return 1;
    std::vector<Journey> trips;
    for (int i = 1; i <= n; ++i) {
        std::cout << "Journey " << i << '\n';
        Journey j{};
        if (!readInt(" zone (1-3): ", 1, 3, j.zone) || !readInt(" hour (0-23): ", 0, 23, j.hour) ||
            !readInt(" minute (0-59): ", 0, 59, j.minute)) {
            std::cout << "\ninput ended early\n";
            break;
        }
        j.peak = isPeak(j.hour * 60 + j.minute);
        long long base = BASE_PAISE[j.zone - 1];
        j.fare = j.peak ? base * 120 / 100 : base;
        trips.push_back(j);
    }

    const std::string line = "+----+--------+-------+------+------------+";
    std::cout << '\n' << line << '\n'
              << "| #  | Zone   | Time  | Peak | Fare       |\n" << line << '\n';
    long long total = 0;
    for (std::size_t i = 0; i < trips.size(); ++i) {
        const Journey& j = trips[i];
        total += j.fare;
        std::cout << "| " << std::setw(2) << i + 1 << " | " << std::left << std::setw(6)
                  << ZONE_NAME[j.zone - 1] << std::right << " | " << std::setfill('0')
                  << std::setw(2) << j.hour << ':' << std::setw(2) << j.minute << std::setfill(' ')
                  << " | " << std::setw(4) << (j.peak ? "yes" : "no") << " | " << std::setw(10)
                  << money(j.fare) << " |\n";
    }
    std::cout << line << '\n' << "| Total" << std::setw(34) << money(total) << " |\n";
    if (!trips.empty())
        std::cout << "  average fare Rs " << std::fixed << std::setprecision(2)
                  << total / 100.0 / trips.size() << '\n';
}
