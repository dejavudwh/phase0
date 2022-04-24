#include <iostream>
#include <vector>
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/node/parse.h"
#include <yaml-cpp/yaml.h>

using namespace std;

int main()
{
    // std::string v("");
    YAML::Node node = YAML::LoadFile("test.yml");
    // std::cout << node;
    std::cout << node["logger"]["appender"]["buffer_size"].IsDefined();
}
