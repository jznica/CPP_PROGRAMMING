## TASK B1 – Implement and Validate the Polymorphic Billing (7 marks)

Files: `billing.hpp` (the whole design of A1, A3, A4, A5), `setup.hpp` (test patients, package catalogue, scheme rates), `bills.txt` (input), `b1_validate.cpp`, `complete_the_code.cpp`, `experiments/`. Compiled with `g++ -std=c++17 -Wall -Wextra` – **no warnings** – and run again under `-fsanitize=address,undefined` – **no findings**.

### B1.1 – Validation: one bill of each kind with each payer

Input `bills.txt`: `OP 1 P101 500 1200`, `IP 2 P102 3 2500 40000 4500`, `DC 3 P103 CATARACT`, `ER 4 P104 2000 3.5 800`. Payers: `SelfPay`; `Insurance("StarHealth", cover 50 000, co-pay 10 %)`; `GovernmentScheme("CMCHIS", OP 1 000, IP 30 000, DC-CATARACT 15 000, ER 5 000)`.

Hand calculation (written before running) and program result:

| Bill | Total by hand | SelfPay covers / patient | Insurance: 90 % of total (≤ 50 000) | Scheme: min(rate, total) | Program |
|---|---|---|---|---|---|
| OP #1 Ravi | 500 + 1 200 = **1 700** | 0 / 1 700 | 1 530 / 170 | 1 000 / 700 | PASS ×3 |
| IP #2 Meena | 3 × 2 500 + 40 000 + 4 500 = **52 000** | 0 / 52 000 | 46 800 / 5 200 | 30 000 / 22 000 | PASS ×3 |
| DC #3 Arun | package CATARACT = **18 000** | 0 / 18 000 | 16 200 / 1 800 | 15 000 / 3 000 | PASS ×3 |
| ER #4 Lakshmi | 2 000 + 3.5 × 800 = **4 800** | 0 / 4 800 | 4 320 / 480 | min(5 000, 4 800) = 4 800 / 0 | PASS ×3 |
| **Day total** | 1 700 + 52 000 + 18 000 + 4 800 = **76 500** | | | | PASS |

Result: **12/12 settlements and the day total match the hand calculation** (`outputs/B1_validate.txt`, screenshot `screenshots/B1_validate.png`). Every bill was reached through a `Bill*` and every payer through a `Payer*`, so all 12 combinations exercised late binding in both hierarchies.

### Complete the Code

The three TODOs were completed without changing any other line:

```cpp
// TODO 1
virtual double total() const = 0;
virtual std::string type() const { return "Bill"; }
// TODO 2
double total() const override { return consultation + tests; }
std::string type() const override { return "OP"; }
// TODO 3
double total() const override { return days * perDay + surgery; }
std::string type() const override { return "IP"; }
```

Output (also clean under ASan):

```text
OP Ravi: 1700
IP Meena: 47500
Total 49200
```

identical to the expected output (1 700 = 500 + 1 200; 47 500 = 3 × 2 500 + 40 000; 49 200 = sum).

**Removing `virtual` from `total()`** (`experiments/complete_nonvirtual_total.cpp`) gives three compile errors:

```text
error: initializer specified for non-virtual method 'double Bill::total() const'
error: 'double OutPatient::total() const' marked 'override', but does not override
error: 'double InPatient::total() const' marked 'override', but does not override
```

Explanation: `= 0` (the *pure-specifier*) is allowed only on a virtual function – a non-virtual function cannot be "filled in later" by a derived class, so declaring it pure is meaningless. Consequently `Bill::total` is no longer a virtual function, and the `override` keyword in the derived classes correctly reports that there is nothing to override. This is exactly why `override` should always be written: without it the derived functions would silently become unrelated functions that *hide* the base one.

### B1.2 – Comparison with the Part A design

The implementation follows the A1/A3/A4/A5 design; the differences are deliberate refinements discovered while coding:

1. **`total()` became `const noexcept`** in the pure virtual declaration, so the compiler forces every override to be `noexcept` (A4.2). The "Complete the Code" version keeps the given signature.
2. **`package()`** was added as a virtual function with a default (`type()`): the government scheme needs a package code, and day-care bills need `"DC-<code>"`. Without it the scheme would have to test the bill's type – the switch we wanted to avoid.
3. **Validation moved into the constructors** (`requireNonNegative`) so that an invalid bill can never exist, which is what allows `total()` to be `noexcept`.
4. **`Bill` copying is deleted** and bills are owned by `std::unique_ptr` inside `BillingDesk` – not shown in the first design, but needed for RAII ownership (A5.2) and to prevent slicing.
5. **`PatientRegistry` and `PackageCatalogue`** were added as small classes so that "missing patient record" and "unknown package" are detected in one place and throw the designed exceptions.
6. The input line format and `parseBill()` were added to feed the system from files (needed for B2).

### B1.3 – Proving runtime binding (removing `virtual` from `total()`)

Because a pure virtual cannot simply lose `virtual` (see above), the experiment `experiments/b1_3_nonvirtual_total.cpp` gives `Bill` an ordinary `total()` returning 0 and builds the same program twice:

| Call | `total()` virtual (`-DUSE_VIRTUAL`) | `total()` not virtual |
|---|---|---|
| direct on objects: `op.total()`, `ip.total()` | 1700, 47500 | 1700, 47500 |
| through `Bill*`: OP Ravi | **1700** | **0** |
| through `Bill*`: IP Meena | **47500** | **0** |
| day total via `Bill*` | **49200** | **0** |

Calls on the objects themselves are unaffected (static and dynamic type are the same). Through a base-class pointer, the non-virtual version binds at compile time to `Bill::total` (the static type of `*b`), so every bill is priced 0 and the day's total is silently wrong – no error, no warning. `type()` (still virtual) kept printing "OP"/"IP", which makes the inconsistency obvious. This proves that only late binding lets one loop over `Bill*` compute the right totals.

### B1.4 – Proving the virtual destructor (ASan)

`experiments/b1_4_nonvirtual_dtor.cpp`: `InPatientBill` owns a `std::vector<double>` of daily charges; it is created with `new` and deleted through `Bill*`.

| Build | Output | AddressSanitizer |
|---|---|---|
| virtual `~Bill` (`-DUSE_VIRTUAL_DTOR`) | `total 47500` / `~InPatientBill ~Bill` | clean, exit 0 |
| non-virtual `~Bill` | `total 47500` / `~Bill` (derived destructor skipped) | **`new-delete-type-mismatch`: size of the allocated type 64 bytes; size of the deallocated type 40 bytes** – abort, exit 1 |
| non-virtual, `ASAN_OPTIONS=new_delete_type_mismatch=0` | same | **LeakSanitizer: Direct leak of 32 byte(s)** – the vector's buffer (4 doubles), allocated in `std::vector` from `main` line 33 |

g++ also warned at compile time (`-Wdelete-non-virtual-dtor`). Interpretation: without the virtual destructor, `delete b` (1) runs only `~Bill`, so the vector member is never destroyed (leak), and (2) passes the size of `Bill` (40 B) to the sized `operator delete` for a 64-byte object (heap corruption risk). Both are undefined behaviour; the virtual destructor fixes both.

### Alternative concept – templates (compile-time polymorphism), one container per type

```text
TEMPLATE <T>
ALGORITHM SumAll(v : vector<T>)            ▷ T is fixed at compile time
BEGIN
    s ← 0
    FOR EACH x IN v DO s ← s + x.total() END FOR    ▷ direct call, can be inlined
    RETURN s
END
dayTotal ← SumAll(opBills) + SumAll(ipBills) + SumAll(dcBills) + SumAll(erBills)
```

| | Virtual functions, one `vector<unique_ptr<Bill>>` | Templates, one `vector<T>` per type |
|---|---|---|
| Time per bill | O(1) + indirect call; 9.84 ns (10 M bills, B3) | O(1), no call at all (inlined, vectorisable); **3.06 ns** – ≈3× faster |
| Space | object + vptr + pointer + heap header per bill | only the fields, contiguous; no vptr. One container per type (k containers) |
| Mixed list in arrival order | yes – one list, any order | **lost**: bills of different types cannot be in one container; the order in which they were raised, "all bills of patient X", sorting by time, one `print` loop – all need extra index structures |
| New bill type | new class only | new class **plus** a new container and a new line in every place that iterates over all types; types must be known at compile time (no bill created from a file at run time without a switch) |
| Code size | one copy of each loop | one instantiation per type |

Templates are the right tool for a hot inner loop over bills of one known type (e.g. totalling 10 M in-patient bills in an analytics job). For the billing system, where bills of all kinds must live together in one ledger in arrival order and new kinds are added yearly, the virtual-function design is required.
