#include <iostream>
#include <thread>

#include "Config.hpp"
#include "Logger.h"

int main()
{
    std::thread t1([]() {
        phase0::Config::lookup<int>("server.addr.aaa", 0, "");
        phase0::Config::LoadFromConf("test.yml");
        while (true)
        {
            auto var = phase0::Config::lookup<int>("server.addr.aaa");
            LOG_INFO("val = %s", var->toString().c_str());
            std::cout << var->toString() << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    });

    t1.join();

    return 0;
}