// billing.hpp - Hospital Billing and Insurance Claims (Assignment 5, CO5)
// Jenica A | 192425003 | DSA01 Object Oriented Programming with C++
//
// Implements the design of Part A:
//   A1  abstract Bill + OutPatientBill, InPatientBill, DayCareBill, EmergencyBill
//   A3  abstract Payer + SelfPay, Insurance, GovernmentScheme
//   A4  exception hierarchy rooted at std::exception, noexcept where nothing can fail
//   A5  BillingDesk: settle() logs ClaimError and rethrows with `throw;`,
//       settleOrFallback() recovers so that no bill is ever lost
#pragma once

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace hb {

// ------------------------------------------------------------------ helpers
inline std::string money(double v)
{
    std::ostringstream os;
    os << std::fixed << std::setprecision(2) << v;
    return os.str();
}

// When true, every destructor prints a trace line (used in B2 / B4 to prove
// that all objects are destroyed even when exceptions are thrown).
inline bool g_traceDestructors = false;

// --------------------------------------------------- A4: exception hierarchy
//  std::exception
//   └─ std::runtime_error
//       └─ BillingError                  any failure of the billing domain
//           ├─ InvalidChargeError        negative / impossible charge or rate
//           ├─ UnknownPackageError       package code not in hospital catalogue
//           ├─ RecordNotFoundError       missing patient record
//           └─ ClaimError                payer rejects the claim
//               └─ CoverExceededError    claim above the insurance cover
class BillingError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};
class InvalidChargeError : public BillingError {
public:
    using BillingError::BillingError;
};
class UnknownPackageError : public BillingError {
public:
    using BillingError::BillingError;
};
class RecordNotFoundError : public BillingError {
public:
    using BillingError::BillingError;
};
class ClaimError : public BillingError {
public:
    using BillingError::BillingError;
};
class CoverExceededError : public ClaimError {
public:
    using ClaimError::ClaimError;
};

inline double requireNonNegative(double v, const std::string& what)
{
    if (v < 0) throw InvalidChargeError("negative " + what + " (" + money(v) + ")");
    return v;
}

// ------------------------------------------------------ A1: bill hierarchy
class Bill {
protected:
    int id_;
    std::string patient_;
    Bill(int id, std::string patient) : id_(id), patient_(std::move(patient)) {}

public:
    Bill(const Bill&) = delete;             // a bill is an entity, not a value
    Bill& operator=(const Bill&) = delete;
    virtual ~Bill()                          // virtual: delete through Bill* is safe
    {
        if (g_traceDestructors) std::cout << "  ~Bill(#" << id_ << ")\n";
    }

    virtual double total() const noexcept = 0;   // pure virtual; never throws (data validated in ctor)
    virtual std::string type() const = 0;    // pure virtual - "OP", "IP", "DC", "ER", ...
    virtual std::string package() const { return type(); }   // virtual, default = type

    virtual void print(std::ostream& os) const               // virtual, overridable
    {
        os << std::left << std::setw(4) << type() << " #" << id_ << ' '
           << std::setw(8) << patient_ << " total " << std::right << std::setw(10)
           << money(total());
    }

    int id() const noexcept { return id_; }
    const std::string& patient() const noexcept { return patient_; }
};

class OutPatientBill : public Bill {
    double consultation_, tests_;

public:
    OutPatientBill(int id, std::string p, double consultation, double tests)
        : Bill(id, std::move(p)),
          consultation_(requireNonNegative(consultation, "consultation")),
          tests_(requireNonNegative(tests, "tests")) {}
    ~OutPatientBill() override
    {
        if (g_traceDestructors) std::cout << "  ~OutPatientBill(#" << id_ << ")\n";
    }
    double total() const noexcept override { return consultation_ + tests_; }
    std::string type() const override { return "OP"; }
};

class InPatientBill : public Bill {
    int days_;
    double perDay_, procedures_, medicines_;

public:
    InPatientBill(int id, std::string p, int days, double perDay, double procedures,
                  double medicines)
        : Bill(id, std::move(p)),
          days_(static_cast<int>(requireNonNegative(days, "days"))),
          perDay_(requireNonNegative(perDay, "room rate")),
          procedures_(requireNonNegative(procedures, "procedures")),
          medicines_(requireNonNegative(medicines, "medicines")) {}
    ~InPatientBill() override
    {
        if (g_traceDestructors) std::cout << "  ~InPatientBill(#" << id_ << ")\n";
    }
    double total() const noexcept override { return days_ * perDay_ + procedures_ + medicines_; }
    std::string type() const override { return "IP"; }
};

class DayCareBill : public Bill {
    std::string code_;
    double packagePrice_;

public:
    DayCareBill(int id, std::string p, std::string code, double packagePrice)
        : Bill(id, std::move(p)), code_(std::move(code)),
          packagePrice_(requireNonNegative(packagePrice, "package price")) {}
    ~DayCareBill() override
    {
        if (g_traceDestructors) std::cout << "  ~DayCareBill(#" << id_ << ")\n";
    }
    double total() const noexcept override { return packagePrice_; }
    std::string type() const override { return "DC"; }
    std::string package() const override { return "DC-" + code_; }
};

class EmergencyBill : public Bill {
    double base_, hours_, perHour_;

public:
    EmergencyBill(int id, std::string p, double base, double hours, double perHour)
        : Bill(id, std::move(p)), base_(requireNonNegative(base, "base charge")),
          hours_(requireNonNegative(hours, "hours")),
          perHour_(requireNonNegative(perHour, "hourly rate")) {}
    ~EmergencyBill() override
    {
        if (g_traceDestructors) std::cout << "  ~EmergencyBill(#" << id_ << ")\n";
    }
    double total() const noexcept override { return base_ + hours_ * perHour_; }
    std::string type() const override { return "ER"; }
};

// ----------------------------------------------------- A3: payer hierarchy
class Payer {
public:
    virtual ~Payer() = default;
    virtual double amountCovered(const Bill& b) const = 0;   // may throw ClaimError
    virtual std::string name() const = 0;
};

class SelfPay : public Payer {
public:
    double amountCovered(const Bill&) const override { return 0.0; }
    std::string name() const override { return "SelfPay"; }
};

class Insurance : public Payer {
    std::string insurer_;
    double coverLimit_, copayPct_;

public:
    Insurance(std::string insurer, double coverLimit, double copayPct)
        : insurer_(std::move(insurer)),
          coverLimit_(requireNonNegative(coverLimit, "cover limit")), copayPct_(copayPct)
    {
        if (copayPct < 0 || copayPct > 100)
            throw InvalidChargeError("co-payment must be 0..100%");
    }
    double amountCovered(const Bill& b) const override
    {
        double claim = b.total() * (100.0 - copayPct_) / 100.0;
        if (claim > coverLimit_)
            throw CoverExceededError("bill #" + std::to_string(b.id()) + ": claim " +
                                     money(claim) + " exceeds cover " + money(coverLimit_));
        return claim;
    }
    std::string name() const override { return "Insurance(" + insurer_ + ")"; }
};

class GovernmentScheme : public Payer {
    std::string scheme_;
    std::map<std::string, double> rates_;   // fixed package rate per package code

public:
    GovernmentScheme(std::string scheme, std::map<std::string, double> rates)
        : scheme_(std::move(scheme)), rates_(std::move(rates)) {}
    double amountCovered(const Bill& b) const override
    {
        auto it = rates_.find(b.package());
        if (it == rates_.end())
            throw ClaimError("bill #" + std::to_string(b.id()) + ": package " + b.package() +
                             " not covered by " + scheme_);
        return std::min(it->second, b.total());   // scheme pays its fixed rate at most
    }
    std::string name() const override { return "Scheme(" + scheme_ + ")"; }
};

// --------------------------------------------- records and package catalogue
class PatientRegistry {
    std::map<std::string, std::string> names_;

public:
    void add(const std::string& pid, const std::string& name) { names_[pid] = name; }
    const std::string& find(const std::string& pid) const
    {
        auto it = names_.find(pid);
        if (it == names_.end()) throw RecordNotFoundError("no patient record for " + pid);
        return it->second;
    }
};

class PackageCatalogue {
    std::map<std::string, double> price_;

public:
    void add(const std::string& code, double price) { price_[code] = price; }
    double price(const std::string& code) const
    {
        auto it = price_.find(code);
        if (it == price_.end()) throw UnknownPackageError("unknown package code " + code);
        return it->second;
    }
};

// Parses one input line:  OP id pid consultation tests
//                         IP id pid days perDay procedures medicines
//                         DC id pid packageCode
//                         ER id pid base hours perHour
// std::stod throws std::invalid_argument for text where a number is expected.
inline std::unique_ptr<Bill> parseBill(const std::string& line, const PatientRegistry& reg,
                                       const PackageCatalogue& cat)
{
    std::istringstream in(line);
    std::string kind, idTok, pid;
    in >> kind >> idTok >> pid;
    std::vector<std::string> f;
    for (std::string t; in >> t;) f.push_back(t);
    auto need = [&](std::size_t n) {
        if (f.size() != n)
            throw InvalidChargeError(kind + " bill needs " + std::to_string(n) + " fields");
    };
    int id = std::stoi(idTok);
    const std::string& name = reg.find(pid);   // RecordNotFoundError
    if (kind == "OP") {
        need(2);
        return std::make_unique<OutPatientBill>(id, name, std::stod(f[0]), std::stod(f[1]));
    }
    if (kind == "IP") {
        need(4);
        return std::make_unique<InPatientBill>(id, name, std::stoi(f[0]), std::stod(f[1]),
                                               std::stod(f[2]), std::stod(f[3]));
    }
    if (kind == "DC") {
        need(1);
        return std::make_unique<DayCareBill>(id, name, f[0], cat.price(f[0]));
    }
    if (kind == "ER") {
        need(3);
        return std::make_unique<EmergencyBill>(id, name, std::stod(f[0]), std::stod(f[1]),
                                               std::stod(f[2]));
    }
    throw InvalidChargeError("unknown bill type " + kind);
}

// ------------------------------------------- A5: catch, log, rethrow, recover
struct Settlement {
    int billId;
    std::string payer;
    double total, covered, patientPays;
    std::string note;
};

class BillingDesk {
    std::vector<std::unique_ptr<Bill>> bills_;   // RAII ownership: bills are never lost
    std::vector<std::string> audit_;

public:
    Bill& add(std::unique_ptr<Bill> b)
    {
        bills_.push_back(std::move(b));
        return *bills_.back();
    }
    const std::vector<std::unique_ptr<Bill>>& bills() const noexcept { return bills_; }
    const std::vector<std::string>& audit() const noexcept { return audit_; }

    // Logs a rejected claim and rethrows the SAME exception object (throw;).
    Settlement settle(const Bill& b, const Payer& p)
    {
        try {
            double covered = p.amountCovered(b);
            double t = b.total();
            return {b.id(), p.name(), t, covered, t - covered, ""};
        } catch (const ClaimError& e) {
            audit_.push_back(p.name() + " rejected: " + e.what());
            std::cout << "  [audit] " << audit_.back() << '\n';
            throw;   // rethrow: dynamic type (e.g. CoverExceededError) is preserved
        }
    }

    // Recovery: a rejected claim is billed to the fallback payer, so the bill survives.
    Settlement settleOrFallback(const Bill& b, const Payer& p, const Payer& fallback)
    {
        try {
            return settle(b, p);
        } catch (const ClaimError& e) {
            Settlement s = settle(b, fallback);
            s.note = std::string("fallback after: ") + e.what();
            return s;
        }
    }

    // A1.3: the day's total through base-class pointers
    double dayTotal() const noexcept
    {
        double sum = 0;
        for (const auto& b : bills_) sum += b->total();   // one virtual call per bill
        return sum;
    }
};

}  // namespace hb
