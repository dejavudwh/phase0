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
        phase0::Logger& logger = phase0::Logger::getInstance();
        logger.setAppender(phase0::LogAppender::ptr(new phase0::AsynFileAppender("log")));
        while (true)
        {
            LOG_INFO("%s", "aa");
            LOG_INFO("%s", "bb");
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    });

    t1.join();
}