// Task 6 Stage 5 - demonstration
#include "fleet.h"
#include "electric_bus.h"     // the only line main needs for the new type

int main()
{
    const Date today(21, 11, 2026);
    Fleet fleet;
    fleet.add(new Bicycle("BC-01", 150));
    fleet.add(new TwoWheeler("TN22AB1234", 500, 80, Date(15, 1, 2027)));
    fleet.add(new Car("TN09CD5678", 2200, 90, Date(10, 3, 2027), 7));
    fleet.add(new ElectricCar("TN01EV0042", 2800, 95, Date(5, 2, 2027), 5, 40));
    fleet.add(new Car("TN10XY0001", 1800, 70, Date(1, 11, 2026), 5));   // service overdue
    fleet.list();

    std::cout << "\n== Renting each type ==\n";
    fleet.rent("BC-01", 2, today);
    fleet.rent("TN22AB1234", 3, today);
    fleet.rent("TN09CD5678", 2, today);
    fleet.rent("TN01EV0042", 4, today);
    fleet.rent("TN10XY0001", 1, today);      // refused: service overdue
    fleet.rent("BC-01", 1, today);           // refused: already rented
    fleet.list();

    std::cout << "\n== Returning (one late) ==\n";
    fleet.giveBack("BC-01", 2, 0);           // on time
    fleet.giveBack("TN22AB1234", 3, 4.5);    // on time, 4.5 litres
    fleet.giveBack("TN09CD5678", 4, 20);     // 2 days late
    fleet.giveBack("TN01EV0042", 4, 60);     // on time, 60 kWh

    std::cout << "\n== Extension: ElectricBus added without changing existing classes ==\n";
    fleet.add(new ElectricBus("TN07EB0900", 9000, 85, Date(20, 12, 2026), 40, 300));
    fleet.rent("TN07EB0900", 1, today);
    fleet.giveBack("TN07EB0900", 2, 180);    // 1 day late, stricter late rate
    fleet.list();
}
