// Task 8 alternative - read whole lines with std::getline and convert with std::stoi
// Compile: g++ -std=c++17 -Wall -Wextra getline_stoi.cpp -o getline_stoi
// Run    : ./getline_stoi < hours.txt
#include <iostream>
#include <stdexcept>
#include <string>

int main()
{
    std::string line;
    int n = 0;
    while (std::getline(std::cin, line)) {
        ++n;
        std::cout << "line " << n << " \"" << line << "\" -> ";
        try {
            std::size_t used = 0;
            int hour = std::stoi(line, &used);
            if (used != line.size())      std::cout << "rejected (trailing characters)\n";
            else if (hour < 0 || hour > 23) std::cout << "rejected (out of range)\n";
            else                          std::cout << "hour " << hour << '\n';
        } catch (const std::invalid_argument&) {
            std::cout << "rejected (not a number)\n";
        } catch (const std::out_of_range&) {
            std::cout << "rejected (too large for int)\n";
        }
    }
}
