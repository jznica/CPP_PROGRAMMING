// What if add() returned Cart (a copy) instead of Cart& ?
#include <iomanip>
#include <iostream>
#include <string>
class Cart {
    std::string items[10]; double prices[10]; int n = 0; double discount = 0;
public:
    Cart add(const std::string& item, double price)          // returns a COPY
    { if (n < 10) { items[n] = item; prices[n] = price; ++n; } return *this; }
    Cart setDiscount(double discount) { this->discount = discount; return *this; }
    double total() const { double t = 0; for (int i = 0; i < n; ++i) t += prices[i]; return t * (100 - discount) / 100; }
};
int main()
{
    Cart c;
    c.add("Rice", 60).add("Dal", 120).setDiscount(10);       // only "Rice" reaches c
    Cart* p = &c;
    p->add("Oil", 180);
    std::cout << std::fixed << std::setprecision(2) << p->total() << '\n';
}
