#include <string>
#include <thread>
#include <vector>

#include "fiber.h"
#include "utils.h"

void runInFiber()
{
    P0ROOT_LOG_INFO() << "runInFiber begin";
    phase0::Fiber::YieldToHold();
    P0ROOT_LOG_INFO() << "runInFiber end";
    phase0::Fiber::YieldToHold();
}

void testFiber()
{
    P0ROOT_LOG_INFO() << "main begin -1";
    {
        phase0::Fiber::GetThis();
        P0ROOT_LOG_INFO() << "main begin";
        phase0::Fiber::ptr fiber(new phase0::Fiber(runInFiber)); //4 5 6
        fiber->swapIn();
        P0ROOT_LOG_INFO() << "main after swapIn";
        fiber->swapIn();
        P0ROOT_LOG_INFO() << "main after end";
        fiber->swapIn();
    }
    P0ROOT_LOG_INFO() << "main after end2";
}

int main(int argc, char** argv)
{
    phase0::SetCurThreadName("Thread");
    P0ROOT_LOG_INFO() << "main";
    std::vector<std::thread> thrs;
    for (int i = 0; i < 3; ++i)
    {
        thrs.push_back(std::thread([=]() {
            phase0::SetCurThreadName("fiber" + std::to_string(i));
            testFiber();
        }));
    }
    for (auto& i : thrs)
    {
        i.join();
    }

    return 0;
}
