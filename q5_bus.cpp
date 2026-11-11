// Q5 - College Bus Seat Tickets: full design + test with more tickets than seats,
// and tickets created inside a function called twice
#include <iostream>
#include <string>

class BusTicket {
    static int issued;                 // belongs to the class: one copy only
    static int waiting;                // number on the waiting list
    static const int CAPACITY = 5;     // integral const static: may be initialised in-class
    int seat;                          // belongs to each object
    std::string name;
public:
    explicit BusTicket(const std::string& n = "Unnamed") : seat(0), name(n)
    {
        if (issued < CAPACITY) seat = ++issued;
        else ++waiting;
    }
    static int count()       { return issued; }      // no 'this' -> only static members
    static int waitCount()   { return waiting; }
    static int seatsLeft()   { return CAPACITY - issued; }
    // static int bad() { return seat; }   // error: invalid use of member 'seat' in static member function
    void print() const
    {
        std::cout << "  " << name << ": "
                  << (seat ? "Seat " + std::to_string(seat) : std::string("Waiting")) << '\n';
    }
};
int BusTicket::issued  = 0;    // definitions: exactly once, outside the class
int BusTicket::waiting = 0;

void bookTrip(const std::string& tag)
{
    BusTicket a(tag + "-1"), b(tag + "-2");   // local objects, destroyed at '}'
    a.print(); b.print();
    std::cout << "  issued so far = " << BusTicket::count() << '\n';
}

int main()
{
    std::cout << "Array of 7 tickets, capacity 5:\n";
    BusTicket trip[7] = {BusTicket("Asha"), BusTicket("Bala"), BusTicket("Chitra"),
                         BusTicket("Dev"), BusTicket("Esha"), BusTicket("Farook"),
                         BusTicket("Gita")};
    for (const BusTicket& t : trip) t.print();
    std::cout << "Issued: " << BusTicket::count() << ", waiting: " << BusTicket::waitCount()
              << ", seats left: " << BusTicket::seatsLeft() << '\n';

    std::cout << "\nLoop creating 2 more:\n";
    for (int i = 0; i < 2; ++i) { BusTicket t("Loop" + std::to_string(i)); t.print(); }

    std::cout << "\nbookTrip() called twice:\n";
    bookTrip("Call1");
    bookTrip("Call2");
    std::cout << "Final issued: " << BusTicket::count() << ", waiting: " << BusTicket::waitCount() << '\n';
}
