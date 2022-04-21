#include <iostream>
#include <vector>

using namespace std;

#define F(name) \
    do          \
    {           \
        f(name); \
    } while (0)

void f(const std::string& name) { std::cout << "string " << name << std::endl; }

void f(const char* name) { std::cout << "char*" << name << std::endl; }

int main() 
{
    string str("asd");
    F(str);
}
