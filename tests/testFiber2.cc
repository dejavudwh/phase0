
#include <string>
#include <vector>

#include "fiber.h"

void run_in_fiber2()
{
    P0ROOT_LOG_INFO() << "run_in_fiber2 begin";
    P0ROOT_LOG_INFO() << "run_in_fiber2 end";
}

void run_in_fiber()
{
    P0ROOT_LOG_INFO() << "run_in_fiber begin";

    /**
     * 非对称协程，子协程不能创建并运行新的子协程，下面的操作是有问题的，
     * 子协程再创建子协程，原来的主协程就跑飞了
     */
    phase0::Fiber::ptr fiber(new phase0::Fiber(run_in_fiber2, 0, false));
    fiber->swapIn();

    P0ROOT_LOG_INFO() << "run_in_fiber end";
}

int main(int argc, char *argv[])
{
    P0ROOT_LOG_INFO() << "main begin";

    phase0::Fiber::GetThis();

    phase0::Fiber::ptr fiber(new phase0::Fiber(run_in_fiber, 0, false));
    fiber->swapIn();

    P0ROOT_LOG_INFO() << "main end";
    return 0;
}