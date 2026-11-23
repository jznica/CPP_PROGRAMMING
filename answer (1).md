## TASK A5 – Design Recovery and Rethrowing (4 marks)

### A5.1 – Where each exception is caught, logged, rethrown and reported

![catch and rethrow flow](diagrams/catchflow.png)

| Exception | Thrown in | Caught / logged | Rethrown? | Recovered / reported |
|---|---|---|---|---|
| `CoverExceededError`, `ClaimError` | `Payer::amountCovered()` (level 1) | `BillingDesk::settle()` (level 2a) writes the reason to the audit trail | **yes, `throw;`** – the same object, so `CoverExceededError` stays a `CoverExceededError` | `settleOrFallback()` (level 2b) recovers: the bill is settled with `SelfPay` and the reason is attached to the settlement; the bill stays in the desk |
| `InvalidChargeError`, `UnknownPackageError`, `RecordNotFoundError`, `std::invalid_argument` | `parseBill()`, constructors, registry, catalogue (level 1) | not caught in between – nothing useful can be done there | – | `main()` (level 4) reports "line N REJECTED (type): reason" and continues with the next line |
| `std::bad_alloc` | any allocation | `main()` | – | reported; the current operation is abandoned, the program continues (or stops cleanly) |
| anything else (`catch(...)`) | e.g. a legacy module | `main()` | – | "unknown exception", program keeps running |

Principles: catch an exception **only where something can be done with it** (log, recover, or report); intermediate functions such as `processLine()` have no `try` at all and rely on RAII for cleanup; log-and-rethrow uses `throw;` so the original type and message reach the outer handler; the top level is the only place that talks to the user, and the loop over input lines is *inside* the top-level `try`, so one bad line never stops the others.

```text
ALGORITHM ProcessFile(lines, desk, payer)
BEGIN
    FOR EACH line IN lines DO
        TRY
            bill ← ParseBill(line)                     ▷ may throw input errors
            ref  ← desk.Add(bill)                      ▷ stored first: never lost
            s    ← desk.SettleOrFallback(ref, payer, SelfPay)
            PRINT s
        CATCH BillingError e        : PRINT "REJECTED", type, e.what()
        CATCH invalid_argument e    : PRINT "REJECTED: text where a number is expected"
        END TRY
    END FOR
END

ALGORITHM Settle(bill, payer)                          ▷ BillingDesk::settle
BEGIN
    TRY
        covered ← payer.AmountCovered(bill)
        RETURN (bill.total, covered, bill.total − covered)
    CATCH ClaimError e
        audit.Append(payer.name + " rejected: " + e.what())
        RETHROW                                        ▷ throw;
    END TRY
END

ALGORITHM SettleOrFallback(bill, payer, fallback)
BEGIN
    TRY
        RETURN Settle(bill, payer)
    CATCH ClaimError e
        s ← Settle(bill, fallback);  s.note ← "fallback after: " + e.what()
        RETURN s
    END TRY
END
```

### A5.2 – No leaks on errors: stack unwinding and RAII

When an exception is thrown, the runtime searches the call stack for a matching handler. Every function between the `throw` and the handler is exited, and **for every fully constructed automatic object in those stack frames the destructor is called, in reverse order of construction** – this is *stack unwinding*. Control then enters the handler. The code after the throwing call in those functions is never executed.

RAII (*Resource Acquisition Is Initialisation*) ties each resource to an object: the constructor acquires it, the destructor releases it. Because destructors run during unwinding, the resource is released on **every** exit path – normal return, `return` in the middle, or exception – without any `catch`:

* `std::unique_ptr<Bill>` deletes the bill (through the virtual destructor) when it goes out of scope;
* `std::vector`, `std::string`, `std::map` free their memory;
* `std::ifstream` closes the file;
* the `BillingDesk` owns its bills in a `vector<unique_ptr<Bill>>`, so destroying the desk destroys all bills.

If a constructor throws, the members and base sub-objects that were already constructed are destroyed (B2 shows `~Bill(#11)` printed when the `OutPatientBill` constructor rejected a negative charge) and the memory from `new`/`make_unique` is freed automatically.

Evidence: the B2 run with destructor traces shows every bill and every RAII guard destroyed even on the error paths, and the AddressSanitizer runs of B2 and B4 (which throw and catch many exceptions) report no leaks.

### Predict the Output

**My prediction (written before running):**

```text
~f ~main caught 42
```
(followed by a newline; no space after 42.)

**Actual output** (`outputs/A5_predict_output.txt`):

```text
~f ~main caught 42
```

**Differences:** none.

**Order of the destructors.** `main` enters the `try` block and constructs `m("main")`; it calls `f()`, which constructs `g("f")` and executes `throw 42`. `f` has no handler, so unwinding starts in `f`: its local `g` is destroyed → `~f `. The search continues in `main`; the throw happened inside `main`'s `try` block, so before control can enter a handler of that block, all automatic objects of the block are destroyed → `~m` prints `~main `. Now the handlers are tried in order: `catch (int x)` matches the thrown `int` → `caught 42`. `catch (...)` is skipped (only the first matching handler runs). Finally `'\n'` is printed after the try/catch. Destruction is in reverse order of construction (`g` was constructed last, so it is destroyed first) and always *before* the handler body runs.

**Why "not printed" never appears.** `throw` transfers control out of `f()` directly to the handler; `f()` never returns normally, so the statement after the call in the `try` block is skipped. Execution never resumes at the point of the throw (C++ exceptions have *termination* semantics, not resumption).

### Alternative concept – `delete` in every catch block instead of RAII

```text
ALGORITHM ProcessManual(billId)
BEGIN
    charges ← new real[1000]                       ▷ raw pointer
    TRY
        SubmitClaim(billId)
    CATCH ClaimError
        delete[] charges;  RETHROW                 ▷ leak point 1
    CATCH RecordNotFoundError                      ▷ must be ADDED when this error is introduced
        delete[] charges;  RETHROW                 ▷ leak point 2
    END TRY
    t ← charges[0];  delete[] charges              ▷ normal path, leak point 3
    RETURN t
END

ALGORITHM ProcessRAII(billId)
BEGIN
    charges ← vector<real>(1000)                   ▷ destructor frees it on every path
    SubmitClaim(billId)
    RETURN charges[0]
END
```

Experiment `TaskA5/alt_manual_delete.cpp` (under `-fsanitize=address`): `SubmitClaim` originally threw only `ClaimError`, and the manual version had a `catch`+`delete[]` for it. Then a **new error path** (`RecordNotFoundError` for bill 3) was added *without* adding a new `catch`. Result: LeakSanitizer – *"Direct leak of 8000 byte(s) in 1 object(s) allocated from processManual(int)"*. The RAII version, unchanged, had no leak. Adding the second `catch` (`-DFIXED`) removed the leak.

| | Manual `delete` in catch blocks | RAII (`unique_ptr`, `vector`) |
|---|---|---|
| Time | same: one allocation and one free on every path; unwinding cost identical | same |
| Code size | one `catch`+`delete` per (resource × exception type) plus the normal-path delete: with *r* resources and *e* exception types ≈ r·(e+1) release statements | 0 release statements; the destructor is written once in the library |
| Places where a leak can appear when one new error path is added | every function on the call chain that owns a resource and does not yet catch that type (here 1 → an actual leak); also every early `return` | **0** – destructors run for any exception type and any `return` |
| Other risks | double delete, delete after rethrow forgotten, `catch(...)` that swallows errors | none of these |

RAII is equal in run time, smaller in code, and makes "no leaks on errors" a property of the type instead of a discipline the programmer must remember at every new error path. It is the only design that scales as the hospital adds new errors and new payers.
