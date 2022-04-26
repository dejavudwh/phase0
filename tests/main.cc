#include <yaml-cpp/yaml.h>

#include <iostream>
#include <memory>
#include <vector>

#include "yaml-cpp/node/node.h"
#include "yaml-cpp/node/parse.h"

using namespace std;

class Bar
{
public:
    int m;
    Bar(int s = 0) {m = s;}
    ~Bar() { std::cout << "destroying" << std::endl; }
};

shared_ptr<Bar> msp;

void testSwap(shared_ptr<Bar> sp) 
{
        std::cout << sp.use_count() << std::endl;

    msp.swap(sp);
}

int main() 
{
    shared_ptr<Bar> sp1 = shared_ptr<Bar>(new Bar(1));
    std::cout << "===" << std::endl;
    testSwap(sp1);
    
    std::cout << sp1.use_count() << std::endl;
    std::cout << msp.use_count() << std::endl;
    std::cout << "===" << std::endl;
}
