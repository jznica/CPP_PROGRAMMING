// Fleet stored as an array of base-class pointers
#pragma once
#include "vehicles.h"

class Fleet {
    static const int MAX = 20;
    Vehicle* vehicles[MAX] = {};
    int count = 0;
public:
    Fleet() = default;
    Fleet(const Fleet&) = delete;
    Fleet& operator=(const Fleet&) = delete;
    ~Fleet() { for (int i = 0; i < count; ++i) delete vehicles[i]; }   // virtual destructor runs

    bool add(Vehicle* v)
    {
        if (count == MAX || v == nullptr) { delete v; return false; }
        vehicles[count++] = v;
        return true;
    }
    Vehicle* find(const std::string& reg) const
    {
        for (int i = 0; i < count; ++i)
            if (vehicles[i]->reg() == reg) return vehicles[i];
        return nullptr;
    }
    void rent(const std::string& reg, int days, const Date& today) const
    {
        Vehicle* v = find(reg);
        std::cout << "RENT   " << std::left << std::setw(13) << reg;
        if (v && v->rent(days, today)) std::cout << "OK for " << days << " day(s)\n";
        else std::cout << "REFUSED (not found, already rented, low level or service overdue)\n";
    }
    void giveBack(const std::string& reg, int actualDays, double usage) const
    {
        Vehicle* v = find(reg);
        std::cout << "RETURN " << std::left << std::setw(13) << reg;
        if (!v || !v->isRented()) { std::cout << "ERROR (not rented)\n"; return; }
        double late = 0;
        std::streamsize oldPrecision = std::cout.precision();
        double bill = v->giveBack(actualDays, usage, late);
        std::cout << std::fixed << std::setprecision(2) << "bill Rs." << bill
                  << "  (late charge Rs." << late << ")\n";
        std::cout.unsetf(std::ios::fixed);
        std::cout.precision(oldPrecision);
    }
    void list() const
    {
        std::cout << "---- Fleet (" << count << " vehicles) ----\n";
        for (int i = 0; i < count; ++i) vehicles[i]->print(std::cout);
    }
};
