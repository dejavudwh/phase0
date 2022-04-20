#include <iostream>

#include "Config.hpp"

int main()
{
    phase0::Config::lookup<int>("server.zxc", 0, "");
    phase0::Config::LoadFromConf("test.yml");
    auto var = phase0::Config::lookup<int>("server.zxc");
    std::cout << var->toString() << std::endl;
}