# Task 1 – Objects in the Namma Metro fare problem

Task 1 is an analysis task, so this folder holds the analysis. The classes it describes are
implemented in `Task07/oop.cpp`, `Task09/metro_fare.cpp` and (revised) `Task12/metro_v2.cpp`.

| Object    | Data                                         | Behaviour                                         | Why it is an object |
|-----------|----------------------------------------------|---------------------------------------------------|---------------------|
| SmartCard | balance in paise (private); later a pass     | canTravel(fare), deduct(fare), recharge(), balance() | Owns the money and the rule that protects it (≥ Rs 50 left, never negative) |
| Passenger | type: Regular / Student / Senior             | concessionPercent()                                | Concession depends only on who travels |
| Journey   | fare zone, minute of day, passenger          | fareZone()                                         | One gate event; a fare is a pure function of it |
| Zone      | Inner, Middle, Outer (enum class)            | index into the fare table                          | Closed set of values; 7 cannot be a zone |
| FareTable | base fares, surcharge %, concessions, minimum balance, peak windows | baseFare(), isPeak(), fare() | Single home of every fare constant |

## Rules every correct journey must satisfy

| #  | Rule | Where enforced |
|----|------|----------------|
| R1 | Fare = Rs 10 / 20 / 30 by the farthest zone travelled | baseFare() |
| R2 | +20 % surcharge for a start time in [08:00, 10:00) ∪ [17:00, 20:00) | isPeak() |
| R3 | Concession (student 25 %, senior 50 % – assumed rates) applied after the surcharge | concessionPercent() |
| R4 | Fare is a whole number of paise, rounded once (half-up) | single integer expression |
| R5 | balance − fare ≥ 5000 paise, otherwise deny and leave the balance unchanged | SmartCard::canTravel() / deduct() |
| R6 | Balance is never negative | private balance + R5; constructor rejects a negative opening balance |
| R7 | Recharge amount > 0 | recharge() |
| R8 | Unknown zone or malformed time is rejected, not guessed | zoneFromNumber(), parseTime() return std::optional |

## OOP principles

- **Encapsulation** – `SmartCard::balance_` is private; only `deduct()`/`recharge()` change it. This is the fix for the negative-balance incident (Task 7).
- **Abstraction** – the gate calls `card.travel(journey)` without knowing paise arithmetic or peak windows.
- **Inheritance / polymorphism** – possible (a `FareRule` hierarchy, pass types), but not needed: the variations are data (percentages), not behaviour.

Procedural vs OO: the algorithm and its O(1) cost per journey are the same; what changes is who is
allowed to change the balance. In the procedural version every deducting function must repeat the
Rs 50 check; in the OO version it exists in exactly one member function.
