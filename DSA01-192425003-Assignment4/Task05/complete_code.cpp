#include <iomanip>
#include <iostream>
#include <string>

class Cart {
    std::string items[10];
    double prices[10];
    int n = 0;
    double discount = 0;              // percent

public:
    // TODO 1 (completed): store the item and price if there is space, return *this for chaining
    Cart& add(const std::string& item, double price)
    {
        if (this->n < 10) {
            this->items[this->n] = item;
            this->prices[this->n] = price;
            ++this->n;
        }
        return *this;
    }

    // TODO 2 (completed): this-> separates the member from the parameter named discount
    Cart& setDiscount(double discount)
    {
        this->discount = discount;
        return *this;
    }

    double total() const
    {
        double t = 0;
        for (int i = 0; i < n; ++i) t += prices[i];
        return t * (100 - discount) / 100;
    }
};

int main()
{
    Cart c;
    c.add("Rice", 60).add("Dal", 120).setDiscount(10);
    Cart* p = &c;                // pointer to an object
    p->add("Oil", 180);
    std::cout << std::fixed << std::setprecision(2) << p->total() << '\n';
}
