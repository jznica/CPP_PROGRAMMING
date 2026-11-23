// B4.1 / B4.2 - new bill type and new payer used through the unchanged BillingDesk.
// Includes thrown-and-caught exceptions so the sanitizer run covers the error paths.
#include "../TaskB1/setup.hpp"
#include "extension.hpp"
#include <cmath>

using namespace hb;

int main()
{
    PatientRegistry reg = makeRegistry();
    PackageCatalogue cat = makeCatalogue();
    BillingDesk desk;
    for (const auto& line : readLines("../TaskB1/bills.txt")) desk.add(parseBill(line, reg, cat));
    desk.add(std::make_unique<TeleConsultBill>(5, "Farhan", 20, 30, 99));   // 20*30+99 = 699

    SelfPay self;
    Insurance star("StarHealth", 50000, 10);
    GovernmentScheme scheme = makeScheme();
    CorporateTieUp corp("TechCorp", 80, 40000);   // 80% of each bill, max 40,000 per bill
    const Payer* payers[] = {&self, &star, &scheme, &corp};

    // hand-computed covered amounts [bill][payer]; -1 = claim rejected -> fallback SelfPay (0)
    const double expect[5][4] = {{0, 1530, 1000, 1360},
                                 {0, 46800, 30000, 40000},
                                 {0, 16200, 15000, 14400},
                                 {0, 4320, 4800, 3840},
                                 {0, 629.1, -1, 559.2}};
    std::cout << "B4.1  4 old + 1 new bill type  x  3 old + 1 new payer\n";
    int pass = 0, n = 0;
    for (std::size_t i = 0; i < desk.bills().size(); ++i) {
        const Bill& b = *desk.bills()[i];
        b.print(std::cout);
        std::cout << '\n';
        for (int j = 0; j < 4; ++j) {
            Settlement s = desk.settleOrFallback(b, *payers[j], self);
            double want = expect[i][j] < 0 ? 0 : expect[i][j];
            bool ok = std::fabs(s.covered - want) < 1e-6 && (expect[i][j] < 0) == !s.note.empty();
            pass += ok;
            ++n;
            std::cout << "     " << std::left << std::setw(22) << s.payer << std::right << " covered "
                      << std::setw(9) << money(s.covered) << "  patient " << std::setw(9)
                      << money(s.patientPays) << "  " << (ok ? "PASS" : "FAIL")
                      << (s.note.empty() ? "" : "  (" + s.note + ")") << '\n';
        }
    }
    std::cout << "Day total: " << money(desk.dayTotal()) << "  expected 77199.00 "
              << (std::fabs(desk.dayTotal() - 77199) < 1e-6 ? "PASS" : "FAIL") << '\n';

    // error paths: exceptions thrown and caught while objects are alive
    std::cout << "\nB4.2  error paths exercised in this run\n";
    int caught = 0;
    try { TeleConsultBill bad(6, "Farhan", -5, 30, 99); }
    catch (const InvalidChargeError& e) { ++caught; std::cout << "  caught InvalidChargeError: " << e.what() << '\n'; }
    try { CorporateTieUp bad("X", 120, 1000); }
    catch (const InvalidChargeError& e) { ++caught; std::cout << "  caught InvalidChargeError: " << e.what() << '\n'; }
    try { desk.settle(*desk.bills()[4], scheme); }
    catch (const ClaimError& e) { ++caught; std::cout << "  caught ClaimError: " << e.what() << '\n'; }
    try { desk.add(parseBill("DC 7 P101 KNEE", reg, cat)); }
    catch (const UnknownPackageError& e) { ++caught; std::cout << "  caught UnknownPackageError: " << e.what() << '\n'; }
    try { desk.add(parseBill("OP 8 P101 abc 10", reg, cat)); }
    catch (const std::invalid_argument& e) { ++caught; std::cout << "  caught std::invalid_argument: " << e.what() << '\n'; }
    std::cout << "  " << caught << "/5 exceptions caught, bills still stored: " << desk.bills().size() << '\n';
    std::cout << pass << '/' << n << " settlements match hand calculation\n";
    return (pass == n && caught == 5) ? 0 : 1;
}
