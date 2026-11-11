// Task 11 alternative - read the fare table from a configuration file at start-up
// Compile: g++ -std=c++17 -Wall -Wextra -O2 config_loader.cpp -o config_loader
// Run    : ./config_loader fares.cfg
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

// Parses KEY = value lines; '#' starts a comment. Rejects negatives, junk and missing keys.
bool loadFares(const std::string& path, std::map<std::string, long long>& out, std::string& err)
{
    std::ifstream in(path);
    if (!in) { err = "cannot open " + path; return false; }
    std::string line;
    int lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        if (auto hash = line.find('#'); hash != std::string::npos) line.erase(hash);
        if (line.find_first_not_of(" \t\r") == std::string::npos) continue;
        auto eq = line.find('=');
        if (eq == std::string::npos) { err = "line " + std::to_string(lineNo) + ": missing '='"; return false; }
        std::string key, rest;
        std::istringstream(line.substr(0, eq)) >> key;
        std::istringstream vs(line.substr(eq + 1));
        long long value;
        if (!(vs >> value) || (vs >> rest)) { err = "line " + std::to_string(lineNo) + ": bad number"; return false; }
        if (value < 0) { err = key + " is negative"; return false; }
        out[key] = value;
    }
    for (const char* k : {"INNER", "MIDDLE", "OUTER", "PEAK_SURCHARGE_PERCENT", "MIN_BALANCE_PAISE"})
        if (!out.count(k)) { err = std::string("missing key ") + k; return false; }
    return true;
}

int main(int argc, char* argv[])
{
    const std::string path = argc > 1 ? argv[1] : "fares.cfg";
    std::map<std::string, long long> fares;
    std::string err;
    auto t0 = std::chrono::steady_clock::now();
    bool ok = loadFares(path, fares, err);
    auto t1 = std::chrono::steady_clock::now();
    if (!ok) { std::cerr << "config error: " << err << '\n'; return 1; }
    for (const auto& [key, value] : fares) std::cout << key << " = " << value << '\n';
    std::cout << "values loaded: " << fares.size() << ", start-up read time: "
              << std::chrono::duration<double, std::micro>(t1 - t0).count() << " microseconds\n";
}
