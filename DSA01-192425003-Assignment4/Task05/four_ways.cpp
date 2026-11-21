// Task 5.1 and 5.2 - Four ways to reach a Cart, this-chaining, array of pointers search
#include <iomanip>
#include <iostream>
#include <string>

class Cart {
    std::string customer;
    std::string items[10]; double prices[10]; int n = 0; double discount = 0;
public:
    explicit Cart(const std::string& customer) { this->customer = customer; }   // this-> resolves name clash
    Cart& add(const std::string& item, double price)
    { if (n < 10) { items[n] = item; prices[n] = price; ++n; } return *this; }
    Cart& setDiscount(double discount) { this->discount = discount; return *this; }
    double total() const { double t = 0; for (int i = 0; i < n; ++i) t += prices[i]; return t * (100 - discount) / 100; }
    const std::string& name() const { return customer; }
    const Cart* self() const { return this; }                 // 'this' is a pointer (const Cart*)
};

void applyFestivalOffer(Cart& ref) { ref.setDiscount(5); }    // (c) reference: no copy, never null

// Task 5.2 - highest total using pointer arithmetic over an array of Cart*
Cart* highest(Cart* const* carts, int count)
{
    Cart* best = nullptr;
    for (Cart* const* q = carts; q != carts + count; ++q)
        if (*q && (!best || (*q)->total() > best->total())) best = *q;
    return best;
}

int main()
{
    std::cout << std::fixed << std::setprecision(2);
    Cart a("Kavya");                                          // (a) directly by name
    a.add("Rice", 60).add("Dal", 120);
    std::cout << "(a) by name      : " << a.name() << " " << a.total() << '\n';

    Cart* p = &a;                                             // (b) pointer with ->
    p->add("Oil", 180);
    std::cout << "(b) pointer ->   : " << p->name() << " " << p->total() << '\n';

    applyFestivalOffer(a);                                    // (c) through a reference
    Cart& r = a;
    std::cout << "(c) reference    : " << r.name() << " " << r.total() << '\n';

    Cart* h = new Cart("Ravi");                               // (d) new ... delete
    h->add("Milk", 30).add("Bread", 45).setDiscount(0);
    std::cout << "(d) new/delete   : " << h->name() << " " << h->total() << '\n';
    std::cout << "this == &a ?     : " << std::boolalpha << (a.self() == &a) << '\n';

    // Task 5.2 - array of pointers to the carts of all active customers
    Cart* active[4] = { &a, h, new Cart("Meena"), new Cart("Arun") };
    active[2]->add("Atta", 320).add("Ghee", 650);
    active[3]->add("Soap", 40);
    Cart* best = highest(active, 4);
    std::cout << "Highest total    : " << best->name() << " " << best->total() << '\n';
    for (int i = 0; i < 4; ++i)                               // indexing form of the same loop
        std::cout << "  active[" << i << "] " << active[i]->name() << " " << (*(active + i))->total() << '\n';

    delete active[2]; delete active[3];
    delete h;                                                 // release what new created
    h = nullptr;
}
