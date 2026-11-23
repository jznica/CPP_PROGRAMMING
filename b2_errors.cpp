// B2 - Engineer error conditions against the billing system of TaskB1.
// Every rejected line is reported with a clear reason; no valid bill is lost;
// destructor traces prove that every object is destroyed.
#include "../TaskB1/setup.hpp"
#include <cstddef>
#include <typeinfo>

using namespace hb;

struct ScopeTrace {   // RAII object living on the stack of processLine()
    std::string what;
    explicit ScopeTrace(std::string w) : what(std::move(w)) {}
    ~ScopeTrace() { std::cout << "  ~ScopeTrace(" << what << ")\n"; }
};

static std::string trim(std::string s)
{
    s.erase(0, s.find_first_not_of(" \t"));
    s.erase(s.find_last_not_of(" \t") + 1);
    return s;
}

// may throw: BillingError family, std::invalid_argument (stod), std::bad_alloc
void processLine(const std::string& raw, BillingDesk& desk, const PatientRegistry& reg,
                 const PackageCatalogue& cat, const Payer& ins, const Payer& scheme,
                 const Payer& self)
{
    ScopeTrace t("line '" + trim(raw.substr(raw.find('|') + 1)) + "'");
    std::string payerTok = trim(raw.substr(0, raw.find('|')));
    const Payer& payer = payerTok == "INS" ? ins : scheme;
    std::unique_ptr<Bill> bill = parseBill(raw.substr(raw.find('|') + 1), reg, cat);
    Bill& b = desk.add(std::move(bill));   // stored BEFORE the claim: never lost
    Settlement s = desk.settleOrFallback(b, payer, self);
    std::cout << "  OK   bill #" << s.billId << ' ' << b.type() << " total " << money(s.total)
              << " -> " << s.payer << " covers " << money(s.covered) << ", patient pays "
              << money(s.patientPays) << (s.note.empty() ? "" : "  [" + s.note + "]") << '\n';
}

[[maybe_unused]] static void allocateHuge()
{
    ScopeTrace t("allocateHuge");
    std::size_t n = std::size_t(1) << 40;   // 2^40 doubles = 8 TiB
    std::cout << "  requesting " << n << " doubles (8 TiB)...\n";
    double* p = new double[n];   // throws std::bad_alloc
    p[0] = 1;                    // not reached
    delete[] p;
}

void legacyModule() { throw 42; }   // throws a non-std exception type

int main()
{
    g_traceDestructors = true;
    std::cout << "== Test 1-5: bad input lines (errors.txt) ==\n";
    {
        PatientRegistry reg = makeRegistry();
        PackageCatalogue cat = makeCatalogue();
        Insurance ins("StarHealth", 50000, 10);
        GovernmentScheme scheme = makeScheme();
        SelfPay self;
        BillingDesk desk;
        int lineNo = 0, rejected = 0;
        for (const auto& line : readLines("errors.txt")) {
            ++lineNo;
            std::cout << "line " << lineNo << ": " << line << '\n';
            try {
                processLine(line, desk, reg, cat, ins, scheme, self);
            } catch (const InvalidChargeError& e) {
                ++rejected;
                std::cout << "  REJECTED (InvalidChargeError): " << e.what() << '\n';
            } catch (const UnknownPackageError& e) {
                ++rejected;
                std::cout << "  REJECTED (UnknownPackageError): " << e.what() << '\n';
            } catch (const RecordNotFoundError& e) {
                ++rejected;
                std::cout << "  REJECTED (RecordNotFoundError): " << e.what() << '\n';
            } catch (const std::invalid_argument& e) {
                ++rejected;
                std::cout << "  REJECTED (std::invalid_argument from " << e.what()
                          << "): text where a number is expected\n";
            }
        }
        std::cout << "summary: " << lineNo << " lines, " << desk.bills().size()
                  << " bills stored, " << rejected << " lines rejected, "
                  << desk.audit().size() << " claims audited, day total "
                  << money(desk.dayTotal()) << '\n';
        std::cout << "-- end of scope: desk and payers destroyed --\n";
    }

    std::cout << "\n== Test 6: memory allocation failure ==\n";
#ifdef __SANITIZE_ADDRESS__
    // GCC 13 AddressSanitizer aborts on a failed operator new (allocation-size-too-big /
    // out-of-memory) instead of throwing std::bad_alloc, so this test runs only in the
    // normal (non-sanitizer) build.
    std::cout << "  skipped under -fsanitize=address (ASan aborts instead of throwing)\n";
#else
    try {
        allocateHuge();
    } catch (const std::bad_alloc& e) {
        std::cout << "  caught std::bad_alloc: " << e.what() << '\n';
    }
#endif

    std::cout << "\n== Test 7: top-level catch(...) ==\n";
    for (int round = 1; round <= 2; ++round) {
        try {
            ScopeTrace t("round " + std::to_string(round));
            auto bill = std::make_unique<EmergencyBill>(90 + round, "Farhan", 1000, 1, 500);
            if (round == 1) legacyModule();
            std::cout << "  round " << round << " finished normally, total "
                      << money(bill->total()) << '\n';
        } catch (const std::exception& e) {
            std::cout << "  std::exception: " << e.what() << '\n';
        } catch (...) {
            std::cout << "  catch(...): unknown exception, program keeps running\n";
        }
    }
    std::cout << "\nprogram finished normally\n";
}
