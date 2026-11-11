// Q3 - Dynamic Constructors and Destructors : Auditorium Seat Booking
// Compile: g++ -std=c++17 -Wall -Wextra -g -fsanitize=address seats.cpp -o seats_asan && ./seats_asan
//          g++ -std=c++17 -Wall -Wextra -O2 seats.cpp -o seats && ./seats   (memory measurements)
#include <algorithm>
#include <iostream>
#include <string>
#if defined(__GLIBC__) && !defined(__SANITIZE_ADDRESS__)
#include <malloc.h>
#define HAVE_MALLINFO 1
#endif

class Event {
    std::string name;
    int rows, seatsPerRow;
    char** seat;          // array of row pointers, each row allocated separately
    int booked;

public:
    // Dynamic constructor: the size of the seat map is decided at run time
    Event(const std::string& n, int r, int s)
        : name(n), rows(r > 0 ? r : 0), seatsPerRow(s > 0 ? s : 0), seat(nullptr), booked(0) {
        if (rows > 0 && seatsPerRow > 0) {
            seat = new char*[rows];                       // 1 allocation for the row pointers
            for (int i = 0; i < rows; ++i) {
                seat[i] = new char[seatsPerRow];          // + 1 allocation per row
                std::fill(seat[i], seat[i] + seatsPerRow, '.');
            }
        }
        std::cout << "  [ctor] " << name << " : " << rows << " x " << seatsPerRow
                  << " seats, " << bytesRequested() << " bytes requested in "
                  << allocations() << " allocations\n";
    }

    // Copying would share (or need to duplicate) the seat map - forbidden here.
    Event(const Event&) = delete;
    Event& operator=(const Event&) = delete;

    // Destructor: release every row, then the row-pointer array; print a closing summary
    ~Event() {
        if (seat) {
            for (int i = 0; i < rows; ++i) delete[] seat[i];
            delete[] seat;
        }
        std::cout << "  [dtor] " << name << " closed: " << booked << " of "
                  << rows * seatsPerRow << " seats booked, " << bytesRequested()
                  << " bytes released\n";
    }

    std::size_t bytesRequested() const {
        return rows == 0 || seatsPerRow == 0
                   ? 0
                   : rows * sizeof(char*) + static_cast<std::size_t>(rows) * seatsPerRow;
    }
    int allocations() const { return (rows == 0 || seatsPerRow == 0) ? 0 : rows + 1; }

    bool valid(int r, int s) const { return r >= 0 && r < rows && s >= 0 && s < seatsPerRow; }

    bool book(int r, int s) {
        if (!valid(r, s)) { std::cout << "    book(" << r << "," << s << "): no such seat\n"; return false; }
        if (seat[r][s] == 'X') { std::cout << "    book(" << r << "," << s << "): already booked\n"; return false; }
        seat[r][s] = 'X'; ++booked;
        std::cout << "    book(" << r << "," << s << "): OK\n";
        return true;
    }
    bool cancel(int r, int s) {
        if (!valid(r, s) || seat[r][s] != 'X') { std::cout << "    cancel(" << r << "," << s << "): nothing to cancel\n"; return false; }
        seat[r][s] = '.'; --booked;
        std::cout << "    cancel(" << r << "," << s << "): OK\n";
        return true;
    }
    void show() const {
        for (int i = 0; i < rows; ++i) {
            std::cout << "    row " << i << " : ";
            for (int j = 0; j < seatsPerRow; ++j) std::cout << seat[i][j];
            std::cout << '\n';
        }
    }
};

#ifdef HAVE_MALLINFO
static std::size_t heapInUse() { return mallinfo2().uordblks; }
#endif

int main() {
    std::cout << "--- 1. Object on the stack (local) ---\n";
    Event concert("Concert(stack)", 3, 5);
    concert.book(0, 1); concert.book(0, 2); concert.book(2, 4);
    concert.book(0, 2);   // double booking
    concert.cancel(0, 1);
    concert.book(5, 0);   // invalid row
    concert.show();

    std::cout << "\n--- 2. Object inside a block ---\n";
    {
        Event seminar("Seminar(block)", 2, 4);
        seminar.book(1, 1);
        std::cout << "  leaving the block...\n";
    }
    std::cout << "  after the block\n";

    std::cout << "\n--- 3. Object on the heap (new / delete) ---\n";
    Event* drama = new Event("Drama(heap)", 2, 3);
    drama->book(1, 2);
    std::cout << "  calling delete...\n";
    delete drama;
    std::cout << "  after delete\n";

    std::cout << "\n--- 4. Event with zero rows ---\n";
    {
        Event empty("Empty(0 rows)", 0, 10);
        empty.book(0, 0);
        empty.show();
    }

#ifdef HAVE_MALLINFO
    std::cout << "\n--- 5. Memory allocated for different seat maps ---\n";
    for (auto rs : {std::pair<int, int>{10, 20}, {100, 200}, {1000, 2000}}) {
        std::size_t before = heapInUse();
        Event* e = new Event("Map " + std::to_string(rs.first) + "x" + std::to_string(rs.second),
                             rs.first, rs.second);
        std::size_t after = heapInUse();
        std::cout << "    measured heap growth (incl. Event object + malloc headers) = "
                  << after - before << " bytes\n";
        delete e;
    }
#endif
    std::cout << "\n--- end of main: local objects destroyed now ---\n";
    return 0;
}
