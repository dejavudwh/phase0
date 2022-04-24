#include <cstdarg>
#include <iostream>
#include <string>
#include <thread>

#include "LogMarco.h"
#include "utils.h"

int main()
{
    phase0::SetCurThreadName("thread0");
    P0ROOT_LOG_INFO() << "THIS INFO LOG";
    P0ROOT_LOG_DEBUG() << "THIS DEBUG LOG";
    P0ROOT_LOG_WARN() << "THIS WARN LOG";
    P0ROOT_LOG_ERROR() << "THIS ERROR LOG";
    P0ROOT_LOG_FATAL() << "THIS FATAL LOG";

    std::thread t1([]() {
        while (true)
        {
            phase0::SetCurThreadName("thread1");
            P0ROOT_LOG_INFO() << "THIS INFO LOG";
            P0ROOT_LOG_DEBUG() << "THIS DEBUG LOG";
            P0ROOT_LOG_WARN() << "THIS WARN LOG";
            P0ROOT_LOG_ERROR() << "THIS ERROR LOG";
            P0ROOT_LOG_FATAL() << "THIS FATAL LOG";
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    });

    t1.join();
}