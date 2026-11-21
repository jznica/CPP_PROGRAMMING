// Extension added later: NEW file only. vehicles.h and fleet.h are not changed.
#pragma once
#include "vehicles.h"

class ElectricBus : public MotorVehicle {
    int seats; double batteryKWh;
public:
    static constexpr double PER_KWH = 10.0;
    static constexpr double DRIVER_PER_DAY = 800.0;
    ElectricBus(std::string reg, double rate, double lvl, Date due, int s, double kwh)
        : MotorVehicle(std::move(reg), rate, lvl, due), seats(s), batteryKWh(kwh) {}
    std::string type() const override { return "ElectricBus"; }
    double charge(int days, double kwh) const override
    {
        return days * (dailyRate + DRIVER_PER_DAY) + kwh * PER_KWH;
    }
    double lateRatePerDay() const override { return dailyRate * 2; }   // stricter late policy
    void details(std::ostream& os) const override
    {
        MotorVehicle::details(os); os << "  " << seats << " seats  " << batteryKWh << " kWh";
    }
};
