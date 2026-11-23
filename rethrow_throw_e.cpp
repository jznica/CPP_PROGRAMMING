// B2 - "Replace throw; with throw e;" experiment.
// The given program is run with throw e; (RETHROW_COPY) and with throw; .
// To make the difference visible, the over-cover case throws CoverExceededError,
// a class DERIVED from ClaimError (as in billing.hpp), and main() has a handler for it.
#include <iostream>
#include <stdexcept>
#include <typeinfo>

class ClaimError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};
class CoverExceededError : public ClaimError {
public:
    using ClaimError::ClaimError;
};

double approve(double billed, double cover)
{
    if (billed < 0 || cover < 0) throw std::invalid_argument("negative amount");
    if (billed > cover) throw CoverExceededError("exceeds cover");
    return billed;
}

double settle(double billed, double cover)
{
    try {
        return approve(billed, cover);
    } catch (const ClaimError& e) {
        std::cout << "audit: " << e.what() << "  (caught object at " << &e << ")\n";
#ifdef RETHROW_COPY
        throw e;   // copies the static type ClaimError: the CoverExceededError part is sliced off
#else
        throw;     // rethrows the original object, dynamic type preserved
#endif
    }
}

int main()
{
#ifdef RETHROW_COPY
    std::cout << "== settle() uses  throw e; ==\n";
#else
    std::cout << "== settle() uses  throw;   ==\n";
#endif
    double cases[][2] = {{40000, 50000}, {60000, 50000}, {-1, 50000}};
    for (auto& c : cases) {
        try {
            double paid = settle(c[0], c[1]);
            std::cout << "approved " << paid << '\n';
        } catch (const CoverExceededError& e) {
            std::cout << "cover exceeded -> route to self-pay: " << e.what() << "  (object at " << &e << ")\n";
        } catch (const ClaimError& e) {
            std::cout << "claim rejected: " << e.what() << "  [dynamic type "
                      << (typeid(e) == typeid(ClaimError) ? "ClaimError" : "derived") << ", object at " << &e << "]\n";
        } catch (const std::exception& e) {
            std::cout << "error: " << e.what() << '\n';
        }
    }
}
