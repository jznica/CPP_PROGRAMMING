## TASK B3 – Virtual Functions vs `switch` vs Exceptions: the Cost (8 marks)

Environment: Intel Xeon @ 2.30 GHz (2 vCPUs, cloud VM), Ubuntu 24.04, g++ 13.3.0, `-std=c++17 -O2 -Wall -Wextra`. Timing with `std::chrono::steady_clock`; each configuration repeated (20 001 passes for n = 1 000, 21 for 10⁶, 7 for 10⁷) and the **median** pass taken; the whole benchmark was run **3 times** and the table reports the median of the 3 runs (all runs are in `outputs/B3_dispatch.txt`; run-to-run spread ≤ ±15 %). Bill kinds are drawn at random (uniform over 4 kinds, fixed seed) so the order is unpredictable, and every method sums the same bills – the totals printed by all methods are identical, which also guards against the optimiser removing the work.

### B3.1 – Correctness on 10 bills

`bench_dispatch.cpp` creates the same 10 random bills (seed 7: 3 IP, 3 DC, 3 ER, 1 OP) as (a) objects of the virtual hierarchy and (b) `BillRec` structs with a `Kind` field and a `switch`:

| # | 1 OP | 2 ER | 3 DC | 4 IP | 5 IP | 6 DC | 7 IP | 8 ER | 9 ER | 10 DC | Sum |
|---|---|---|---|---|---|---|---|---|---|---|---|
| virtual | 6169.81 | 7962.94 | 8962.81 | 59143.09 | 27943.54 | 24028.51 | 67054.18 | 32235.41 | 11415.68 | 30726.41 | 275642.39 |
| switch | 6169.81 | 7962.94 | 8962.81 | 59143.09 | 27943.54 | 24028.51 | 67054.18 | 32235.41 | 11415.68 | 30726.41 | 275642.39 |

**10/10 bills agree exactly** (bit-for-bit, `==` on doubles, because both compute the same expression in the same order). For n = 10⁶ and 10⁷ the totals of all five methods also agree (templates differ only in the 15th significant digit because they add in a different order).

### B3.2 – Time as the data grows (ns per bill, median of 3 runs)

| n | virtual | switch | `std::variant` (alt.) | `dynamic_cast` (A2 alt.) | templates (B1 alt.) |
|---|---|---|---|---|---|
| 1 000 | 2.09 ns (0.0021 ms) | 2.08 ns | 2.08 ns | 26.15 ns | 1.73 ns |
| 1 000 000 | 8.99 ns (9.0 ms) | 6.48 ns (6.5 ms) | 6.37 ns (6.4 ms) | 31.31 ns | 2.02 ns |
| 10 000 000 | **9.84 ns (98.4 ms)** | **7.77 ns (77.7 ms)** | **8.15 ns (81.5 ms)** | 30.45 ns (304.5 ms) | 3.06 ns (30.6 ms) |

Space per element (printed by the program): virtual objects 16–40 B (OP 24, IP 40, DC 16, ER 32, each including the 8-byte vptr) **plus** an 8-byte `unique_ptr` and a 16-byte heap header; `BillRec` 32 B; `std::variant` 40 B; contiguous in both.

**Interpretation.**

* **Growth is linear (O(n))** for every method: from 10⁶ to 10⁷ bills the time per pass grows ≈10× and the time per bill stays nearly constant.
* **At n = 1 000 virtual, switch and variant are identical (≈2 ns).** The 1 000 bills (≈56 KB) stay in cache, and because the same 1 000-step sequence is repeated 20 001 times the CPU's branch predictor learns the sequence of call targets. The dispatch mechanism then costs almost nothing.
* **At 10⁶–10⁷ bills the virtual version is ≈25–40 % slower than the switch** (9.8 vs 7.8 ns). Two effects: (1) the bill kind is random, so both the indirect call and the switch's jump are mispredicted about 3 times in 4 – this is why both cost far more than at n = 1 000; (2) the virtual version must first follow the pointer to a separately allocated object and then read its vptr – an extra dependent memory load per bill – whereas `BillRec`s lie contiguously in one array and are prefetched.
* **`dynamic_cast` (≈30 ns) is 3× slower** than a virtual call even in cache: each cast is a library call that compares `type_info`, and an ER bill needs 4 casts.
* **Templates (≈2–3 ns)** are fastest: no dispatch and no branches at all, the loops can be inlined and vectorised; the cost is the price of separate containers (B1 alternative).
* In absolute terms the difference between virtual and switch is **2 ns per bill = 0.4 ms for the hospital group's 200 000 bills per day.**

### B3.3 – Cost of exceptions (validate 1 000 000 bills)

`bench_exceptions.cpp`: invalid bill = a negative consultation charge, placed at random. `validate` functions are `noinline` so each design pays a real call. Median of 21 repetitions, median of 3 runs:

| Invalid | Return code (`enum Status`) | `std::optional` | Exceptions | Exception cost per invalid bill |
|---|---|---|---|---|
| 0 % | 1.49 ms (1.5 ns/bill) | 2.58 ms | **2.18 ms** | – |
| 1 % (9 980 invalid) | 1.34 ms | 2.51 ms | **12.80 ms** | (12.80 − 1.34) ms / 9 980 = **1.15 µs** |
| 50 % (500 231 invalid) | 5.09 ms | 5.26 ms | **525.0 ms** | (525.0 − 5.09) ms / 500 231 = **1.04 µs** |

All three designs reject exactly the same bills and produce the same sum (checked by the program).

**Interpretation.** With **no errors**, exceptions cost about the same as the alternatives (≈2 ns per bill – g++'s table-based "zero-cost" model executes no extra instructions for `try`; the ≈0.7 ns gap to the return code comes from how the result is returned, not from exception handling). Every **thrown** exception costs ≈1 µs – about 500× the cost of validating a good bill – because the runtime allocates the exception object, walks the unwind tables to find a handler (two-phase search), runs the destructors and builds the `runtime_error` message string. So the cost is proportional to the *number of errors*: +10 ms for 1 % errors, but 100× slower overall at 50 %. The return code also slows down at 50 % (1.5 → 5 ns) because the success/failure branch becomes unpredictable.

### B3.4 – Engineering trade-off: when to use each approach

| Approach | Use it when | Avoid it when |
|---|---|---|
| **Virtual functions** | the set of types is open (new bill types / payers every year), objects of different types must live in one collection, the code that uses them must not change when types are added; cost ≈ 10 ns per call is small compared with the work per object | a hot loop over millions of small objects where nanoseconds matter and the set of types is fixed |
| **`switch` on a type field** | a small, closed, stable set of types; data-oriented code that needs contiguous storage and the last 20 % of speed (e.g. a batch job); serialisation formats | types are added often – every switch must be found and edited |
| **`std::variant` + `std::visit`** | closed set of types but you want type safety and contiguous storage; the compiler checks that every alternative is handled | the set of types grows, or alternatives differ greatly in size (every element is as big as the largest) |
| **Exceptions** | errors are rare and must travel several levels with a reason; constructors must be able to fail; resource cleanup is via RAII | errors are frequent and expected (≥ several % of inputs in a hot loop) – there, return a status / `optional` locally and convert to an exception at the boundary |
| **Return codes / `optional`** | high-rate, expected failures handled by the immediate caller; `noexcept` or real-time code | the error must cross many layers (every layer needs checking code) |

For the hospital (200 000 bills/day, a few % invalid, two new bill types and three new payers a year) the measured costs are: dispatch 200 000 × 9.84 ns ≈ 2 ms/day; exceptions at a pessimistic 5 % invalid 10 000 × 1.1 µs ≈ 11 ms/day. Performance does not decide the design; extensibility and correctness do.

### Alternative concept – `std::variant<OutPatientBill, InPatientBill, …>` + `std::visit`

```text
TYPE BillVar = variant<OP, IP, DC, ER>          ▷ plain structs, no virtual functions
bills : vector<BillVar>                         ▷ stored by value, contiguous
sum ← 0
FOR EACH b IN bills DO
    sum ← sum + visit(λ(x): x.total(), b)       ▷ jump table on the stored index
END FOR
```

Measured for 10 000 000 bills (median of 3 runs): **variant 81.5 ms (8.15 ns/bill)** vs virtual 98.4 ms (9.84 ns) vs switch 77.7 ms (7.77 ns). `std::visit` with a single variant compiles to a jump table on the index, so it behaves like the `switch` – but generated and checked by the compiler (an alternative without `total()` is a compile error, unlike a forgotten `case`).

Space per element = size of the largest alternative + tag, rounded to alignment: `IPv` (int + 3 doubles) = 32 B, + 1-byte index → **40 B** (`sizeof(BillVar) = 40`), for *every* element – a 16-byte `DCv` also occupies 40 B (60 % wasted for day-care bills). The virtual design uses only what each object needs (16–40 B) but adds an 8-byte pointer and heap overhead per bill and scatters objects in memory.

Extensibility: adding a bill type changes the `variant` type itself, so every function that names it and every `visit` site must be recompiled (and every non-generic visitor edited) – it is a *closed* set like the switch. `std::variant` is therefore the best choice when the types are fixed and speed/locality matter; it is not the right choice for this hospital, whose set of bill types grows every year.
