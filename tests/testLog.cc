#include <cstdarg>
#include <iostream>
#include <string>
#include <thread>

#include "AsynFileAppender.h"
#include "LogAppender.h"
#include "Logger.h"

int main()
{
    std::thread t1([]() {
        while (true)
        {
            // LOG_INFO("%s", str);
            LOG_DEBUG("%s", "basdsadsadasdsab");
            LOG_DEBUG("%s", "cvbbvnbvnmnnm");
            LOG_DEBUG("%s", "zzzzzzzzzzzzzzxczxc");

            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    });

    t1.join();
}