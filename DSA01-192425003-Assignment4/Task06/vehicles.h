// Task 6 - Vehicle rental fleet: existing classes (never edited when new types are added)
#pragma once
#include <iomanip>
#include <iostream>
#include <string>

class Date {                                        // member class (has-a)
    int d, m, y;
public:
    Date(int dd, int mm, int yy) : d(dd), m(mm), y(yy) {}
    bool before(const Date& o) const { return y != o.y ? y < o.y : m != o.m ? m < o.m : d < o.d; }
    std::string str() const
    {
        auto two = [](int v) { return (v < 10 ? "0" : "") + std::to_string(v); };
        return two(d) + "-" + two(m) + "-" + std::to_string(y);
    }
};

class Vehicle {                                     // abstract base class
    std::string regNo;
    bool rented = false;
    int plannedDays = 0;
protected:
    double dailyRate;
public:
    Vehicle(std::string reg, double rate) : regNo(std::move(reg)), dailyRate(rate) {}
    virtual ~Vehicle() = default;
    Vehicle(const Vehicle&) = delete;               // a vehicle is never copied (no slicing)
    Vehicle& operator=(const Vehicle&) = delete;

    virtual std::string type() const = 0;                         // pure virtual
    virtual double charge(int days, double usage) const = 0;      // pure virtual: differs per type
    virtual bool canRent(const Date&) const { return !rented; }
    virtual double lateRatePerDay() const { return dailyRate * 1.5; }
    virtual void details(std::ostream&) const {}

    const std::string& reg() const { return regNo; }
    bool isRented() const { return rented; }

    bool rent(int days, const Date& today)
    {
        if (days <= 0 || !canRent(today)) return false;
        rented = true; plannedDays = days;
        return true;
    }
    double giveBack(int actualDays, double usage, double& lateCharge)
    {
        int lateDays = actualDays > plannedDays ? actualDays - plannedDays : 0;
        lateCharge = lateDays * lateRatePerDay();
        double bill = charge(plannedDays, usage) + lateCharge;   // virtual call
        rented = false; plannedDays = 0;
        return bill;
    }
    void print(std::ostream& os) const
    {
        os << std::left << std::setw(12) << type() << std::setw(13) << regNo << "Rs."
           << std::setw(6) << dailyRate << "/day  " << std::setw(10) << (rented ? "RENTED" : "available");
        details(os);
        os << '\n';
    }
};

class Bicycle : public Vehicle {
public:
    using Vehicle::Vehicle;
    std::string type() const override { return "Bicycle"; }
    double charge(int days, double) const override { return days * dailyRate; }
};

class MotorVehicle : public Vehicle {               // still abstract (no charge())
protected:
    double level;                                   // fuel or charge level in percent
    Date serviceDue;
public:
    MotorVehicle(std::string reg, double rate, double lvl, Date due)
        : Vehicle(std::move(reg), rate), level(lvl), serviceDue(due) {}
    bool canRent(const Date& today) const override
    {
        return Vehicle::canRent(today) && !serviceDue.before(today) && level >= 25;
    }
    void details(std::ostream& os) const override
    {
        os << " level " << level << "%  service " << serviceDue.str();
    }
};

class TwoWheeler : public MotorVehicle {
public:
    static constexpr double PETROL_PER_LITRE = 105.0;
    using MotorVehicle::MotorVehicle;
    std::string type() const override { return "TwoWheeler"; }
    double charge(int days, double litres) const override { return days * dailyRate + litres * PETROL_PER_LITRE; }
};

class Car : public MotorVehicle {
protected:
    int seats;
public:
    static constexpr double PETROL_PER_LITRE = 105.0;
    static constexpr double LARGE_CAR_PER_DAY = 300.0;   // surcharge above 5 seats
    Car(std::string reg, double rate, double lvl, Date due, int s)
        : MotorVehicle(std::move(reg), rate, lvl, due), seats(s) {}
    std::string type() const override { return "Car"; }
    double seatSurcharge(int days) const { return seats > 5 ? days * LARGE_CAR_PER_DAY : 0; }
    double charge(int days, double litres) const override
    {
        return days * dailyRate + seatSurcharge(days) + litres * PETROL_PER_LITRE;
    }
    void details(std::ostream& os) const override { MotorVehicle::details(os); os << "  " << seats << " seats"; }
};

class ElectricCar : public Car {                    // multilevel: Vehicle->MotorVehicle->Car->ElectricCar
    double batteryKWh;
public:
    static constexpr double PER_KWH = 12.0;
    ElectricCar(std::string reg, double rate, double lvl, Date due, int s, double kwh)
        : Car(std::move(reg), rate, lvl, due, s), batteryKWh(kwh) {}
    std::string type() const override { return "ElectricCar"; }
    double charge(int days, double kwh) const override        // per kWh instead of per litre
    {
        if (kwh > batteryKWh * days) kwh = batteryKWh * days;  // cannot bill above capacity
        return days * dailyRate + seatSurcharge(days) + kwh * PER_KWH;
    }
    void details(std::ostream& os) const override { Car::details(os); os << "  " << batteryKWh << " kWh"; }
};
