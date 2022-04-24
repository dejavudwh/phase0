#include <iostream>
#include <list>
#include <map>
#include <thread>
#include <vector>

#include "Config.hpp"
#include "LogMarco.h"

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
            P0ROOT_LOG_DEBUG() <<testInt->toString();

            auto testList = phase0::Config::lookup<list<float>>("server.test_list", {0.0, 0.0, 0.0}, "");
            P0ROOT_LOG_DEBUG() << testList->toString();

            auto testMap = phase0::Config::lookup<map<string, string>>(
                "server.test_map", {{"k1", "v1"}, {"k2", "v2"}}, "");
            P0ROOT_LOG_DEBUG() << testMap->toString();

            auto testCombine = phase0::Config::lookup<map<string, vector<int>>>(
                "server.test_combine", {{"k1", {0, 0, 0}}, {"k2", {0, 0, 0}}}, "");
            P0ROOT_LOG_DEBUG() << testCombine->toString();
            
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    });

    t1.join();

    return 0;
}