// Q3 experiment: (a) forgetting delete, (b) delete instead of delete[]
// Compile: g++ -std=c++17 -Wall -Wextra -g -fsanitize=address bugs.cpp -o bugs
// Run    : ./bugs leak      ./bugs mismatch      ./bugs correct
#include <cstring>
#include <iostream>

struct SeatMap {
    int rows, seats;
    char** seat;
    SeatMap(int r, int s) : rows(r), seats(s), seat(new char*[r]) {
        for (int i = 0; i < rows; ++i) seat[i] = new char[seats]();
    }
    ~SeatMap() {
        for (int i = 0; i < rows; ++i) delete[] seat[i];
        delete[] seat;
    }
};

int main(int argc, char** argv) {
    const char* mode = argc > 1 ? argv[1] : "correct";
    if (std::strcmp(mode, "leak") == 0) {
        SeatMap* m = new SeatMap(3, 4);   // created on the heap ...
        (void)m;                          // ... but delete m; is forgotten
        std::cout << "leak: program ends without delete\n";
    } else if (std::strcmp(mode, "mismatch") == 0) {
        char* row = new char[20];
        std::cout << "mismatch: allocated with new[], releasing with delete\n";
        delete row;                       // WRONG: should be delete[] row;
    } else {
        SeatMap* m = new SeatMap(3, 4);
        delete m;
        char* row = new char[20];
        delete[] row;
        std::cout << "correct: every new has a matching delete / delete[]\n";
    }
    return 0;
}
