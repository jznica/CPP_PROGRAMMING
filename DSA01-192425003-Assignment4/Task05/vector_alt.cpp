// Task 5 - Alternative: std::vector<Cart> (objects) vs array of Cart*; pointer invalidation on growth
#include <iostream>
#include <string>
#include <vector>
struct Cart { std::string items[10]; double prices[10] = {}; int n = 0; double discount = 0;
              double total() const { double t = 0; for (int i = 0; i < n; ++i) t += prices[i]; return t; } };
int main()
{
    std::cout << "sizeof(Cart) = " << sizeof(Cart) << " bytes per object in vector<Cart>\n";
    std::cout << "sizeof(Cart*) = " << sizeof(Cart*) << " bytes per slot in Cart* array\n";
    std::vector<Cart> v(1);
    v.reserve(1);
    const Cart* first = &v[0];
    std::cout << "capacity before = " << v.capacity() << '\n';
    v.emplace_back();                                         // grows -> reallocates
    std::cout << "capacity after  = " << v.capacity() << '\n';
    std::cout << "old pointer still valid? " << std::boolalpha << (first == &v[0])
              << "  (false = dangling pointer)\n";
}
