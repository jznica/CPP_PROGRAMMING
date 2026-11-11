// Q5 - Overloading Increment, Comparison and Subscript : Music Playlist and Track Timer
// Compile: g++ -std=c++17 -Wall -Wextra player.cpp -o player && ./player
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

class Time {
    int h, m, s;
public:
    Time(int hh = 0, int mm = 0, int ss = 0) : h(hh), m(mm), s(ss) {
        m += s / 60; s %= 60;   // normalise, e.g. Time(0, 0, 75) -> 1:15
        h += m / 60; m %= 60;
    }
    // prefix ++t : advance one second, return the object itself
    Time& operator++() {
        if (++s == 60) { s = 0; if (++m == 60) { m = 0; ++h; } }
        return *this;
    }
    // postfix t++ : the int is only a tag; save old value, advance, return the OLD copy
    Time operator++(int) {
        Time old = *this;
        ++*this;
        return old;
    }
    long seconds() const { return h * 3600L + m * 60L + s; }
    bool operator==(const Time& o) const { return seconds() == o.seconds(); }
    bool operator<(const Time& o) const { return seconds() < o.seconds(); }
    friend std::ostream& operator<<(std::ostream& os, const Time& t) {
        char f = os.fill('0');
        if (t.h > 0) os << t.h << ':' << std::setw(2) << t.m;
        else os << t.m;
        os << ':' << std::setw(2) << t.s;
        os.fill(f);
        return os;
    }
};

struct Track {
    std::string title;
    Time length;
};

class Playlist {
    std::vector<Track> tracks;
    static Track& invalidTrack() {       // returned when the index is out of range
        static Track t;
        t = Track{"<invalid track>", Time()};
        return t;
    }
    bool inRange(int i) const { return i >= 0 && i < static_cast<int>(tracks.size()); }
public:
    void add(const Track& t) { tracks.push_back(t); }
    int size() const { return static_cast<int>(tracks.size()); }

    // non-const version: playlist[i] can be modified (e.g. rename a track)
    Track& operator[](int i) {
        if (!inRange(i)) {
            std::cerr << "  error: index " << i << " out of range [0, " << size() - 1 << "]\n";
            return invalidTrack();
        }
        return tracks[i];
    }
    // const version: called on a const Playlist, read-only access
    const Track& operator[](int i) const {
        if (!inRange(i)) {
            std::cerr << "  error: index " << i << " out of range [0, " << size() - 1 << "]\n";
            return invalidTrack();
        }
        return tracks[i];
    }
};

void printAll(const Playlist& p) {                // uses the const operator[]
    for (int i = 0; i < p.size(); ++i)
        std::cout << "  " << i << ". " << p[i].title << " (" << p[i].length << ")\n";
}

int main() {
    std::cout << "--- 1. Prefix vs postfix ---\n";
    Time t(0, 0, 10);
    Time a = t++;
    std::cout << "  after a = t++ : a = " << a << ", t = " << t << '\n';
    Time b = ++t;
    std::cout << "  after b = ++t : b = " << b << ", t = " << t << '\n';

    std::cout << "\n--- 2. Roll-over ---\n";
    Time r1(0, 0, 59);  std::cout << "  " << r1 << " -> "; ++r1; std::cout << r1 << '\n';
    Time r2(0, 59, 59); std::cout << "  " << r2 << " -> "; ++r2; std::cout << r2 << '\n';
    Time r3(1, 59, 59); std::cout << "  " << r3 << " -> "; r3++; std::cout << r3 << '\n';

    std::cout << "\n--- 3. Comparisons ---\n";
    std::cout << std::boolalpha << "  Time(0,1,0) == Time(0,0,60) : " << (Time(0, 1, 0) == Time(0, 0, 60))
              << "\n  Time(0,3,59) < Time(0,4,0)  : " << (Time(0, 3, 59) < Time(0, 4, 0))
              << "\n  Time(1,0,0)  < Time(0,59,59): " << (Time(1, 0, 0) < Time(0, 59, 59)) << '\n';

    Playlist list;
    list.add({"Kanmani Anbodu", Time(0, 0, 4)});
    list.add({"Vaathi Coming", Time(0, 0, 3)});
    list.add({"Malare", Time(0, 0, 5)});
    std::cout << "\n--- 4. Playlist ---\n";
    printAll(list);
    list[1].title = "Vaathi Coming (Remix)";      // non-const operator[]
    std::cout << "  after rename: list[1] = " << list[1].title << '\n';

    std::cout << "\n--- 5. Out-of-range access ---\n";
    std::string t1 = list[-1].title;
    std::cout << "  list[-1]   -> " << t1 << '\n';
    std::string t2 = list[list.size()].title;
    std::cout << "  list[size] -> " << t2 << '\n';

    std::cout << "\n--- 6. Playback simulation (track lengths shortened to seconds) ---\n";
    Time clock;
    for (int i = 0; i < list.size(); ++i) {
        const Track& tr = list[i];
        Time pos;
        std::cout << "  Playing " << tr.title << " [" << tr.length << "] : ";
        while (pos < tr.length) { std::cout << pos << ' '; ++pos; ++clock; }
        std::cout << "| done at " << pos << '\n';
    }
    std::cout << "  total play time = " << clock << '\n';
}
