// Without virtual: accessing a Person member through TA is ambiguous
#include <iostream>
#include <string>
struct Person { std::string name = "Arun"; };
struct Student : Person {};
struct Staff : Person {};
struct TeachingAssistant : Student, Staff {};
int main()
{
    TeachingAssistant ta;
    std::cout << ta.name << '\n';            // error: request for member 'name' is ambiguous
}
