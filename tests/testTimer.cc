#include "iomanager.h"


static int timeout = 1000;
static phase0::Timer::ptr s_timer;

void timer_callback() {
    P0ROOT_LOG_INFO() << "timer callback, timeout = " << timeout;
    timeout += 1000;
    if(timeout < 5000) {
        s_timer->reset(timeout, true);
    } else {
        s_timer->cancel();
    }
}

void test_timer() {
    phase0::IOManager iom;

    // 循环定时器
    s_timer = iom.addTimer(1000, timer_callback, true);
    
    // 单次定时器
    iom.addTimer(500, []{
        P0ROOT_LOG_INFO() << "500ms timeout";
    });
    iom.addTimer(5000, []{
        P0ROOT_LOG_INFO() << "5000ms timeout";
    });
}

int main(int argc, char *argv[]) {
    test_timer();

    P0ROOT_LOG_INFO() << "end";

    return 0;
}