## TASK B2 – Engineer Error Conditions (5 marks)

Program `b2_errors.cpp` reuses the unchanged B1 classes and reads `errors.txt`, a file of deliberately constructed bad input (payer | bill line). Destructor tracing is switched on (`g_traceDestructors = true`) and a small RAII guard `ScopeTrace` lives on the stack of `processLine()`, so every destruction is visible. Run normally and under `-fsanitize=address`.

### Results (`outputs/B2_errors.txt`)

| Line | Input | Observed behaviour | What it reveals about the design |
|---|---|---|---|
| 1 | `INS │ OP 10 P101 500 1200` | OK – insurance covers 1 530, patient pays 170 | control case: normal path unaffected by the error handling |
| 2 | `INS │ OP 11 P101 -500 1200` | **REJECTED (InvalidChargeError): negative consultation (-500.00)**; trace shows `~Bill(#11)` then `~ScopeTrace` before the handler | the constructor refuses the invalid bill; the already-built `Bill` base sub-object is destroyed and the memory from `make_unique` is freed – no half-built bill exists, nothing is stored |
| 3 | `INS │ IP 12 P102 30 2500 40000 4500` (total 119 500) | `[audit] … claim 107550.00 exceeds cover 50000.00`, then **OK → SelfPay, patient pays 119 500 [fallback after: …]** | claim above cover: `Insurance` throws `CoverExceededError`; `settle()` logs and rethrows with `throw;`; `settleOrFallback()` recovers. **The bill is not lost** |
| 4 | `SCHEME │ DC 13 P103 KNEE` | **REJECTED (UnknownPackageError): unknown package code KNEE** | catalogue lookup fails before any bill is created |
| 5 | `INS │ ER 14 P104 2000 abc 800` | **REJECTED (std::invalid_argument from stod)**: text where a number is expected | library exceptions pass through our code untouched and are reported at the top level; the line number tells the user where |
| 6 | `SCHEME │ OP 15 P999 300 0` | **REJECTED (RecordNotFoundError): no patient record for P999** | missing record detected in one place (`PatientRegistry::find`) |
| 7 | `SCHEME │ DC 16 P105 HERNIA` | audit + **fallback to SelfPay (package DC-HERNIA not covered by CMCHIS)** | a scheme rejection is a `ClaimError` – recovered the same way as the insurance case without any extra code |
| 8 | `SCHEME │ ER 17 P104 1500 2 600` | OK – scheme covers 2 700 (bill below the 5 000 rate) | processing continues normally after six errors |

Summary printed by the program: **8 lines, 4 bills stored, 4 lines rejected, 2 claims audited, day total 158 900.00** (= 1 700 + 119 500 + 35 000 + 2 700 ✓). At the end of the scope the trace shows all four stored bills destroyed, each derived destructor followed by `~Bill`, in insertion order (the `vector` destroys its elements first to last).

**Test 6 – memory allocation failure.** `new double[2^40]` (8 TiB) → `~ScopeTrace(allocateHuge)` printed during unwinding, then **caught `std::bad_alloc`**. The RAII guard proves the function's locals were cleaned up even though the allocation itself failed.
*Finding:* under `-fsanitize=address` (GCC 13) a failed `operator new` does **not** throw – ASan aborts with *allocation-size-too-big* (or *out-of-memory* with `allocator_may_return_null=1`). The test therefore runs only in the normal build; the sanitizer build prints "skipped" for it. All other tests run under ASan with **no findings** (`outputs/B2_errors_asan.txt`).

**Test 7 – top-level `catch(...)`.** Round 1 calls a "legacy module" that throws an `int` (not derived from `std::exception`). The trace shows `~EmergencyBill(#91)`, `~Bill(#91)`, `~ScopeTrace(round 1)` – the `unique_ptr`-owned bill and the guard are destroyed – then `catch(...): unknown exception, program keeps running`. Round 2 completes normally and the program ends with "program finished normally". `catch(...)` is the last safety net; it cannot tell *what* went wrong, which is why all our own errors derive from `std::exception`.

**What the tests show overall:** (1) each error is detected at the earliest point that has the information, with a message naming the field, value, bill or patient; (2) the exception *type* selects the right reaction – input errors are reported, claim errors are recovered; (3) a bill is stored before its claim is attempted, so claim failures never lose it; (4) RAII destroys every object on every path, and ASan confirms no leaks; (5) one bad line never stops the run.

### Predict the Output

**My prediction (written before running):**

```text
ok done caught negative [log too large] caught too large 
```
(with a trailing space, then a newline)

* `100.0` – valid: `validate` prints `ok `, `process` returns, `main` prints `done `.
* `-5.0` – `validate` throws `std::invalid_argument`. `process`'s handler is for `std::out_of_range`; `invalid_argument` and `out_of_range` are *siblings* (both derive from `std::logic_error`), so it does not match. The exception leaves `process`, `done ` is skipped, and `main`'s `catch (const std::exception&)` matches → `caught negative `.
* `2e6` – `validate` throws `std::out_of_range`; `process`'s handler matches → `[log too large] `, then `throw;` rethrows the same object; `main` catches it as `std::exception` (binding by reference, `what()` is still "too large") → `caught too large `.

**Actual output** (`outputs/B2_predict_output.txt`): `ok done caught negative [log too large] caught too large ` – identical; no differences.

**Which handler caught each exception:** `-5` → only `main`'s `std::exception` handler; `2e6` → first `process`'s `out_of_range` handler (log), then, after `throw;`, `main`'s `std::exception` handler. Only one exception was logged because `process` catches *only* `out_of_range`; handlers are matched by type (exact type or a public base class, through the reference), not by "any exception". The `invalid_argument` passed straight through `process` without being seen.

### Complete the Code

```cpp
// TODO 1
if (billed < 0 || cover < 0) throw std::invalid_argument("negative amount");
if (billed > cover) throw ClaimError("exceeds cover");
return billed;
// TODO 2
std::cout << "audit: " << e.what() << '\n';
throw;
```

Output – identical to the expected output:

```text
approved 40000
audit: exceeds cover
claim rejected: exceeds cover
error: negative amount
```

(The negative case is not audited because `settle` catches only `ClaimError`; `invalid_argument` goes straight to `main`'s `std::exception` handler.)

**Replacing `throw;` with `throw e;`.** In the given program the output does not change (`complete_the_code_throw_e.cpp`), because the thrown object *is exactly* a `ClaimError`. The difference is in what is thrown:

* `throw;` rethrows **the original exception object** – same address, same dynamic type.
* `throw e;` **copy-initialises a new exception object** from `e`, whose type is the *static* type of `e`, i.e. `ClaimError`. If the original was a derived class, the derived part is **sliced off**.

To make it visible, `rethrow_throw_e.cpp` throws `CoverExceededError` (derived from `ClaimError`, as in our real design) and `main` has a handler for it:

| | `throw;` | `throw e;` |
|---|---|---|
| over-cover case caught by | `catch (const CoverExceededError&)` → "route to self-pay" | `catch (const ClaimError&)` → "claim rejected", dynamic type is plain `ClaimError` |
| object address in `settle` vs in `main` | same (`0x…c340` in both) – one object | different (`0x…b340` → `0x…b410`) – a copy was made |

So `throw e;` silently destroys information the outer handler needs (and costs a copy). Always rethrow with `throw;`.

### Alternative concept – `std::optional<double>` instead of exceptions

```text
ALGORITHM Approve(billed, cover) → optional<real>
BEGIN
    IF billed < 0 OR cover < 0 OR billed > cover THEN RETURN nullopt END IF   ▷ "rejected", reason lost
    RETURN billed
END
FOR EACH claim DO
    r ← Approve(claim)
    IF r HAS VALUE THEN pay(r.value) ELSE reject(claim) END IF
END FOR
```

Measured in `TaskB3/bench_exceptions.cpp`, 1 000 000 claims, -O2, median of 3 runs × 21 repetitions:

| Invalid claims | `std::optional` | exceptions | exception cost per invalid claim |
|---|---|---|---|
| 0 % | 2.58 ms | 2.18 ms | – |
| 1 % (≈ 10 000) | 2.51 ms | 12.80 ms | ≈ 1.15 µs |
| 50 % (≈ 500 000) | 5.26 ms | 525.0 ms | ≈ 1.04 µs |

Time complexity is O(n) for both, but the constant on the error path differs by ~3 orders of magnitude: an optional costs the same whether valid or not (a flag test), a throw costs ≈1 µs (allocation of the exception object, two-phase unwinding through the tables, handler matching). With no errors both cost ≈2 ns per claim. Extra space per result: `std::optional<double>` is **16 bytes** instead of 8 (a `bool` flag + padding) for *every* result, valid or not; an exception costs nothing per result and only allocates the exception object (16 B + message) when one is thrown. The optional also **carries no reason** – "exceeds cover" and "negative amount" become the same empty value, so the audit message required by the case study would need a second channel (e.g. `std::variant<double, Error>` or C++23 `std::expected`). Conclusion: `optional` for high-rate, expected rejections in a tight loop; exceptions for rare errors that must carry a reason to a distant handler.
