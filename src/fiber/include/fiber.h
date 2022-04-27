#pragma once

#include <ucontext.h>

#include <atomic>
#include <functional>
#include <memory>

#include "LogMarco.h"

namespace phase0
{
class Fiber : public std::enable_shared_from_this<Fiber>
{
public:
    using ptr = std::shared_ptr<Fiber>;

    enum State
    {
        INIT,
        HOLD,
        EXEC,
        TERM,
        READY,
        EXCEPT,
    };

    Fiber(std::function<void()> cb, size_t stacksize = 0, bool runInScheduler = true);
    ~Fiber();

    void reset(std::function<void()> cb);
    void swapIn();
    void swapOut();

    uint64_t getId() const { return m_id; }
    State getState() const { return m_state; }

public:
    static void SetThis(Fiber* f);
    static Fiber::ptr GetThis();

    static void YieldToReady();
    static void YieldToHold();

    static uint64_t TotalFibers();
    static uint64_t GetFiberId();

    static void MainFunc();

private:
    Fiber();

private:
    uint64_t m_id = 0;
    uint32_t m_stacksize = 0;
    State m_state = READY;
    ucontext_t m_ctx;
    void* m_stack = nullptr;
    std::function<void()> m_cb;

    bool m_runInScheduler;
};

}  // namespace phase0