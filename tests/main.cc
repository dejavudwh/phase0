#include <iostream>
#include <vector>
#include <yaml-cpp/yaml.h>

using namespace std;

int main() 
{
    YAML::Node nodes = YAML::Load("[1,2,3]");
        std::vector<int> vec;
        for(size_t i = 0; i < nodes.size(); ++i) {
            std::cout << nodes[i];
        }
}
