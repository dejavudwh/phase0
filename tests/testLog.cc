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
            std::string str("string asd");
            // LOG_INFO("%s", str);
            LOG_DEBUG("%s", "basdsadsadasdsab");
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    });

    t1.join();
}