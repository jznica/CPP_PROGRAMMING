# DSA01 – Assignment 4 (CO4): Designing Reusable Class Hierarchies

| | |
|---|---|
| **Name** | Jenica A |
| **Register number** | 192425003 |
| **Course** | Object Oriented Programming with C++ (DSA01) |
| **Submission date** | 21-11-2026 |
| **Compiler** | g++ (GCC) 13.3.0, C++17 |

## Compile command

```bash
g++ -std=c++17 -Wall -Wextra -o <program> <program>.cpp
./<program>
```

Sanitizer runs (Task 4): `g++ -std=c++17 -Wall -Wextra -g -fsanitize=address -o buggy_asan buggy.cpp`
Benchmark (Task 6 alternative): add `-O2`.

Every folder holds the C++ sources and an `out_*.txt` file with the exact command and the captured output. Output screenshots go next to them.

## Tasks and results

| Folder | Files | How to run | Result |
|---|---|---|---|
| Task01 | `staff_hierarchy.cpp`, `staff_switch_alt.cpp` | compile and run each | Abstract `Staff` base, `SalariedStaff` holds the DA rule once; one loop pays all 4 categories (total 249200.00). Switch version: 72 bytes/object vs 56–64 for derived classes. |
| Task02 | `predict.cpp`, `predict_nonvirtual.cpp`, `university.cpp`, `composition_alt.cpp`, `ambiguity_demo.cpp` | compile and run each (`ambiguity_demo.cpp` is meant to fail) | Virtual base: `Person Student Staff TA`. Without `virtual`: `Person Student Person Staff TA`, and `ta.name` is ambiguous. One Person in TA (168 B) vs two with composition (224 B). |
| Task03 | `trace.cpp`, `trace_nonvirtual.cpp`, `predict.cpp`, `complete_code.cpp`, `complete_code_swapped.cpp`, `pointer_member_alt.cpp` | compile and run each | Order: virtual bases → direct bases → members → body; destruction is the reverse. Predict: `Vehicle Engine Car \| ~Car ~Engine ~Vehicle`. Complete the Code prints `192311001 Kavya (19) Salem-636001`; swapped list gives `-Wreorder` warnings. |
| Task04 | `buggy.cpp`, `fixed.cpp`, `predict.cpp` | normal build, then the `-fsanitize=address` build | ASan reports new-delete-type-mismatch and an 800-byte leak in the buggy version; the fixed version prints 3.5 / 4 and is sanitizer-clean. Predict: `3.5 4 3.5`. |
| Task05 | `complete_code.cpp`, `return_by_value.cpp`, `four_ways.cpp`, `vector_alt.cpp` | compile and run each | `324.00` as expected; returning `Cart` by value gives `240.00`. Four access methods, chaining, and an array-of-pointers max search (Meena 970.00). Vector growth leaves old pointers dangling. |
| Task06 | `vehicles.h`, `fleet.h`, `electric_bus.h`, `main.cpp`, `switch_alt.cpp` | `g++ -std=c++17 -Wall -Wextra -o rental main.cpp && ./rental` | Rents every type, refuses an overdue-service car, returns a car 2 days late (Rs.6600 late charge), then adds `ElectricBus` in a new header with no edits to existing classes. ASan-clean. |
