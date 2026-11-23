// B1.1 - one bill of each kind settled with each payer, checked against hand totals
#include "setup.hpp"
#include <cmath>

using namespace hb;

int main(int argc, char** argv)
{
    const std::string file = argc > 1 ? argv[1] : "bills.txt";
    PatientRegistry reg = makeRegistry();
    PackageCatalogue cat = makeCatalogue();
    BillingDesk desk;
    for (const auto& line : readLines(file)) desk.add(parseBill(line, reg, cat));

    SelfPay self;
    Insurance star("StarHealth", 50000, 10);   // cover 50,000, co-pay 10%
    GovernmentScheme scheme = makeScheme();
    const Payer* payers[] = {&self, &star, &scheme};   // base-class pointers

    // hand-computed {covered, patientPays} for [bill][payer]
    const double expect[4][3][2] = {
        {{0, 1700}, {1530, 170}, {1000, 700}},          // OP 500+1200            = 1700
        {{0, 52000}, {46800, 5200}, {30000, 22000}},    // IP 3*2500+40000+4500   = 52000
        {{0, 18000}, {16200, 1800}, {15000, 3000}},     // DC CATARACT package     = 18000
        {{0, 4800}, {4320, 480}, {4800, 0}}};           // ER 2000+3.5*800        = 4800

    std::cout << "B1.1  bill x payer settlement (polymorphic Bill* and Payer*)\n";
    std::cout << "-------------------------------------------------------------------------------\n";
    int pass = 0, n = 0;
    for (std::size_t i = 0; i < desk.bills().size(); ++i) {
        const Bill& b = *desk.bills()[i];
        b.print(std::cout);
        std::cout << '\n';
        for (int j = 0; j < 3; ++j) {
            Settlement s = desk.settle(b, *payers[j]);
            bool ok = std::fabs(s.covered - expect[i][j][0]) < 1e-9 &&
                      std::fabs(s.patientPays - expect[i][j][1]) < 1e-9;
            pass += ok;
            ++n;
            std::cout << "     " << std::left << std::setw(22) << s.payer << std::right
                      << " covered " << std::setw(10) << money(s.covered) << "  patient "
                      << std::setw(10) << money(s.patientPays) << "  "
                      << (ok ? "PASS" : "FAIL") << '\n';
        }
    }
    std::cout << "-------------------------------------------------------------------------------\n";
    std::cout << "Day total (A1.3 loop over Bill*): " << money(desk.dayTotal())
              << "   expected 76500.00 " << (desk.dayTotal() == 76500 ? "PASS" : "FAIL") << '\n';
    std::cout << pass << '/' << n << " settlements match hand calculation\n";
    return pass == n ? 0 : 1;
}
