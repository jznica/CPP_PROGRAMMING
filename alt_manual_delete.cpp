// A5 alternative - releasing memory with delete in every catch block instead of RAII.
// Build with -DFIXED for the version where the new error path also deletes.
//   g++ -std=c++17 -Wall -Wextra -g -fsanitize=address alt_manual_delete.cpp
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

struct ClaimError : std::runtime_error { using std::runtime_error::runtime_error; };
struct RecordNotFoundError : std::runtime_error { using std::runtime_error::runtime_error; };

void submitClaim(int billId)
{
    if (billId == 2) throw ClaimError("claim above cover");
    if (billId == 3) throw RecordNotFoundError("missing patient record");   // NEW error path
}

// ---- manual version: every exit path must remember to delete
double processManual(int billId)
{
    double* charges = new double[1000];   // owned by a raw pointer
    charges[0] = 500;
    try {
        submitClaim(billId);
    } catch (const ClaimError& e) {
        std::cout << "  manual: " << e.what() << " -> delete[] in catch\n";
        delete[] charges;                   // leak point 1 covered
        throw;
    }
#ifdef FIXED
    catch (const RecordNotFoundError&) {
        std::cout << "  manual: missing record -> delete[] in second catch\n";
        delete[] charges;                   // must be added for EVERY new exception type
        throw;
    }
#endif
    double t = charges[0];
    delete[] charges;                       // normal path
    return t;
}

// ---- RAII version: the destructor of std::vector releases memory on every path
double processRAII(int billId)
{
    std::vector<double> charges(1000);
    charges[0] = 500;
    submitClaim(billId);                    // any exception: vector destroyed during unwinding
    return charges[0];
}

int main()
{
    std::cout << std::unitbuf;   // unbuffered: keeps output order with the leak report
    for (int id = 1; id <= 3; ++id) {
        try { double t = processManual(id); std::cout << "bill " << id << " manual ok " << t << '\n'; }
        catch (const std::exception& e) { std::cout << "bill " << id << " manual failed: " << e.what() << '\n'; }
        try { double t = processRAII(id); std::cout << "bill " << id << " RAII   ok " << t << '\n'; }
        catch (const std::exception& e) { std::cout << "bill " << id << " RAII   failed: " << e.what() << '\n'; }
    }
}
