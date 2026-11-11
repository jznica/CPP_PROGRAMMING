# DSA01-192425003-Assignment1

**Design, Implement and Validate the Fare Logic of a City Metro Smart-Card System (CO1)**

| | |
|---|---|
| Name | Jenica A |
| Register number | 192425003 |
| Programme | B.Tech Artificial Intelligence and Machine Learning, Saveetha School of Engineering (SIMATS) |
| Compiler | g++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0, Linux x86-64 |
| Compile command | `g++ -std=c++17 -Wall -Wextra -o prog prog.cpp` (add `-O2` for Task10 totals, Task11 and Benchmarks) |

## Design assumptions (the brief leaves these open)

- Fare zone = farthest zone travelled; gate keypad numbers zones 1 = Inner, 2 = Middle, 3 = Outer.
- Peak hours are half-open: [08:00, 10:00) and [17:00, 20:00). 08:00 and 17:00 are peak; 10:00 and 20:00 are off-peak.
- Concession: student 25 %, senior citizen 50 % (named constants), applied after the peak surcharge.
- All money is `long long` paise; a fare is rounded once, half-up: `(base·(100+s)·(100−c) + 5000) / 10000`.
- A journey is allowed only if `balance − fare ≥ 5000` paise; a recharge must be positive; the balance never goes negative.

## Repository layout

One folder per task. Each folder holds the C++ sources, any input files, the recorded output of every run (`*_output.txt`: the command, compiler messages and program output) and a `screenshots/` folder with a terminal screenshot of each run.

```
Task01/  analysis.md                       objects, rules, OOP principles
Task02/  constants_demo.cpp                one set of constexpr constants, #define vs constexpr, enum class
Task03/  types_demo.cpp                    sizeof / ranges, paise vs double rupees
Task04/  predict.cpp, fixed.cpp            implicit conversions; brace-init and static_cast version
Task05/  predict.cpp, expressions.cpp      precedence, peak/surcharge/concession expressions
Task06/  complete.cpp, boundary.cpp, pseudocode.txt
Task07/  procedural.cpp, oop.cpp           global balance vs SmartCard class
Task08/  gate_io.cpp, getline_stoi.cpp, input_bad.txt, hours.txt
Task09/  metro_fare.cpp                    full fare logic + 32 tests
Task10/  totals.cpp, warnings_demo.cpp     long long / double / float / int totals; -Wall -Wextra warnings
Task11/  config_loader.cpp, fares.cfg      fare table read from a configuration file
Task12/  impact_demo.cpp, metro_v2.cpp     Airport zone, weekend discount, monthly pass + 24 tests
Benchmarks/ benchmarks.cpp                 timings used in the complexity analyses
run_all.sh                                 rebuilds and reruns everything
```

## How to run

Run everything (Linux / WSL / macOS with g++):

```
bash run_all.sh
```

Or one program at a time, from inside its folder:

| Task | Compile and run | Result |
|---|---|---|
| 2 | `g++ -std=c++17 -Wall -Wextra constants_demo.cpp -o constants_demo && ./constants_demo` | constexpr peak fare 1200 paise; the `#define` version gives 100000 (no precedence protection) |
| 2 | `g++ -std=c++17 -Wall -Wextra -DSHOW_ERROR -c constants_demo.cpp` | expected error: `cannot convert 'Zone' to 'int'` (enum class) |
| 3 | `g++ -std=c++17 -Wall -Wextra types_demo.cpp -o types_demo && ./types_demo` | int is not enough for a network day; truncating rupees×100 is wrong for 4586 of 100000 amounts |
| 4 | `g++ -std=c++17 -Wall -Wextra predict.cpp -o predict && ./predict` | `3 3.5 / C 68 / 1 true / s >= u` with a -Wsign-compare warning; prediction matched |
| 4 | `g++ -std=c++17 -Wall -Wextra fixed.cpp -o fixed && ./fixed` | explicit version prints `s < u`; with `-DSHOW_ERROR` narrowing becomes a compile error |
| 5 | `g++ -std=c++17 -Wall -Wextra predict.cpp -o predict && ./predict` | `11 16 / 9 6 4 / 6 4 / 3 1 -3 -1 / 18`; prediction matched |
| 5 | `g++ -std=c++17 -Wall -Wextra expressions.cpp -o expressions && ./expressions` | expression and lookup table agree at all 8 boundary minutes; `i = i++` warning |
| 6 | `g++ -std=c++17 -Wall -Wextra complete.cpp -o complete && ./complete` | matches the expected output exactly |
| 6 | `g++ -std=c++17 -Wall -Wextra boundary.cpp -o boundary && ./boundary` | 10:00 and 20:00 off-peak, 08:00 and 17:00 peak |
| 7 | `g++ -std=c++17 -Wall -Wextra procedural.cpp -o procedural && ./procedural` | procedural balance goes to −600 |
| 7 | `g++ -std=c++17 -Wall -Wextra oop.cpp -o oop && ./oop` | the same charge is rejected; with `-DSHOW_ERROR` a direct write does not compile |
| 8 | `g++ -std=c++17 -Wall -Wextra gate_io.cpp -o gate_io && ./gate_io < input_bad.txt` | recovers from `three`, `abc`, `x9`, range error on 5; total Rs 78.00 |
| 8 | `g++ -std=c++17 -Wall -Wextra getline_stoi.cpp -o getline_stoi && ./getline_stoi < hours.txt` | getline + stoi also rejects `2x` and out-of-range values |
| 9 | `g++ -std=c++17 -Wall -Wextra metro_fare.cpp -o metro_fare && ./metro_fare` | 32 passed, 0 failed |
| 10 | `g++ -std=c++17 -Wall -Wextra -O2 totals.cpp -o totals && ./totals` | long long exact; float off by Rs 82,125 at 3M; int overflows at journey 1,301,505 |
| 10 | `g++ -std=c++17 -Wall -Wextra warnings_demo.cpp -o warnings_demo` | the five warnings discussed in the report |
| 11 | `g++ -std=c++17 -Wall -Wextra -O2 config_loader.cpp -o config_loader && ./config_loader fares.cfg` | 6 values loaded in microseconds |
| 12 | `g++ -std=c++17 -Wall -Wextra impact_demo.cpp -o impact_demo && ./impact_demo` | -Wswitch warning; Airport costs 0 paise in the unchanged v1 switch |
| 12 | `g++ -std=c++17 -Wall -Wextra metro_v2.cpp -o metro_v2 && ./metro_v2` | 24 passed, 0 failed – decision: MODIFY |
| – | `g++ -std=c++17 -Wall -Wextra -O2 benchmarks.cpp -o benchmarks && ./benchmarks` | timings for Tasks 5, 6, 8 and 10 |

Timings depend on the machine, so they will not match the report digit for digit; the ratios are what matter. All other outputs are deterministic.
