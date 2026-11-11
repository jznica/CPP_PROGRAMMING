#include <iostream>
int main()
{
    int a = 7, b = 2;
    double c = a / b; double d = static_cast<double>(a) / b;
    std::cout << c << ' ' << d << '\n';
    char ch = 'A' + 2;
    std::cout << ch << ' ' << ch + 1 << '\n';
    bool flag = 5;
    std::cout << flag << ' ' << std::boolalpha << flag << '\n';
    unsigned int u = 1;
    int s = -1;
    std::cout << (s < u ? "s < u" : "s >= u") << '\n';
}
