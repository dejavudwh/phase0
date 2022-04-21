#include <iostream>
#include <list>
#include <map>
#include <thread>
#include <vector>

#include "Config.hpp"
#include "Logger.h"

using namespace std;

int main()
{
    std::thread t1([]() {
        phase0::Config::lookup<vector<int>>("server.test_vec", {0, 0, 0}, "");
        phase0::Config::lookup<list<float>>("server.test_list", {0.0, 0.0, 0.0}, "");
        phase0::Config::lookup<map<string, string>>("server.test_map", {{"k1", "v1"}, {"k2", "v2"}}, "");
        phase0::Config::lookup<map<string, vector<int>>>(
            "server.test_combine", {{"k1", {0, 0, 0}}, {"k2", {0, 0, 0}}}, "");

        phase0::Config::LoadFromConf("test.yml");
        while (true)
        {
            auto testInt = phase0::Config::lookup<vector<int>>("server.test_vec", {0, 0, 0}, "");
            LOG_DEBUG("%s", testInt->toString().c_str());

            auto testList = phase0::Config::lookup<list<float>>("server.test_list", {0.0, 0.0, 0.0}, "");
            LOG_DEBUG("%s", testList->toString().c_str());

            auto testMap = phase0::Config::lookup<map<string, string>>(
                "server.test_map", {{"k1", "v1"}, {"k2", "v2"}}, "");
            LOG_DEBUG("%s", testMap->toString().c_str());

            auto testCombine = phase0::Config::lookup<map<string, vector<int>>>(
                "server.test_combine", {{"k1", {0, 0, 0}}, {"k2", {0, 0, 0}}}, "");
            LOG_DEBUG("%s", testCombine->toString().c_str());
            
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    });

    t1.join();

    return 0;
}