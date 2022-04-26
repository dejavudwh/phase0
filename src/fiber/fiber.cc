#include "fiber.h"

#include "Config.hpp"
#include "utils.h"

namespace phase0
{
static std::atomic<uint64_t> FiberId{0};
static std::atomic<uint64_t> FiberCount{0};

static thread_local Fiber* t_fiber = nullptr;
static thread_local Fiber::ptr t_threadFiber = nullptr;

static ConfigVar<uint32_t>::ptr DefaultFiberStackSize =
    Config::lookup<uint32_t>("fiber.stack_size", 128 * 1024, "fiber stack size");

class MallocStackAllocator
{
public:
    static void* Alloc(size_t size) { return malloc(size); }
    static void Dealloc(void* vp, size_t size) { return free(vp); }
};

using StackAllocator = MallocStackAllocator;

Fiber::Fiber()
{
    m_state = EXEC;
    SetThis(this);

    if (getcontext(&m_ctx))
    {
        PHASE0_ASSERT2(false, "Getcontext failed!");
    }

    m_id = ++FiberCount;

    P0SYS_LOG_DEBUG() << "Create main Fiber::Fiber: " << m_id;
}

Fiber::Fiber(std::function<void()> cb, size_t stacksize, bool useCaller) : m_id(++FiberId), m_cb(cb)
{
    m_id = ++FiberCount;
    m_stacksize = stacksize ? stacksize : DefaultFiberStackSize->getValue();

    m_stack = StackAllocator::Alloc(m_stacksize);
    if (getcontext(&m_ctx))
    {
        PHASE0_ASSERT2(false, "Getcontext failed!");
    }
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    if (!useCaller)
    {
        makecontext(&m_ctx, &Fiber::MainFunc, 0);
    }
    else
    {
        makecontext(&m_ctx, &Fiber::CallerMainFunc, 0);
    }

    P0SYS_LOG_DEBUG() << "Create Fiber::Fiber id = " << m_id;
}

Fiber::~Fiber()
{
    --FiberCount;
    if (m_stack)
    {
        PHASE0_ASSERT(m_state == TERM || m_state == EXCEPT || m_state == INIT);

        StackAllocator::Dealloc(m_stack, m_stacksize);
    }
    else
    {
        // main fiber
        PHASE0_ASSERT(!m_cb);
        PHASE0_ASSERT(m_state == EXEC);

        Fiber* cur = t_fiber;
        if (cur == this)
        {
            SetThis(nullptr);
        }
    }
    P0SYS_LOG_DEBUG() << "Fiber::~Fiber id=" << m_id << " total=" << FiberCount;
}

void Fiber::reset(std::function<void()> cb)
{
    PHASE0_ASSERT(m_stack);
    PHASE0_ASSERT(m_state == TERM || m_state == EXCEPT || m_state == INIT);

    m_cb = cb;

    if (getcontext(&m_ctx))
    {
        PHASE0_ASSERT2(false, "Getcontext failed!");
    }

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    makecontext(&m_ctx, &Fiber::MainFunc, 0);
    m_state = INIT;
}

void Fiber::call()
{
    SetThis(this);
    m_state = EXEC;
    if (swapcontext(&t_threadFiber->m_ctx, &m_ctx))
    {
        PHASE0_ASSERT2(false, "Swapcontext falied!");
    }
}

void Fiber::back()
{
    SetThis(t_threadFiber.get());
    if (swapcontext(&m_ctx, &t_threadFiber->m_ctx))
    {
        PHASE0_ASSERT2(false, "Swapcontext failed!");
    }
}

void Fiber::swapIn()
{
    SetThis(this);
    PHASE0_ASSERT(m_state != EXEC);
    m_state = EXEC;
    if (swapcontext(&t_threadFiber->m_ctx, &m_ctx))
    {
        PHASE0_ASSERT2(false, "Swapcontext failed!");
    }
}

void Fiber::swapOut()
{
    SetThis(t_fiber);
    if (swapcontext(&m_ctx, &t_threadFiber->m_ctx))
    {
        PHASE0_ASSERT2(false, "Swapcontext failed!");
    }
}

void Fiber::SetThis(Fiber* f) { t_fiber = f; }

Fiber::ptr Fiber::GetThis()
{
    if (t_fiber)
    {
        return t_fiber->shared_from_this();
    }
    Fiber::ptr mainFiber(new Fiber());

    PHASE0_ASSERT(t_fiber == mainFiber.get());

    t_threadFiber = mainFiber;
    return t_fiber->shared_from_this();
}

void Fiber::YieldToReady()
{
    Fiber::ptr cur = GetThis();

    PHASE0_ASSERT(cur->m_state == EXEC);

    cur->m_state = READY;
    P0SYS_LOG_DEBUG() << "Fiber:" << cur->m_id << " switch1 READY";
    cur->swapOut();
}

void Fiber::YieldToHold()
{
    Fiber::ptr cur = GetThis();

    PHASE0_ASSERT(cur->m_state == EXEC);

    cur->m_state = HOLD;
    P0SYS_LOG_DEBUG() << "Fiber:" << cur->m_id << " switch1 HOLD";
    cur->swapOut();
}

uint64_t Fiber::TotalFibers() { return FiberCount; }

uint64_t Fiber::GetFiberId()
{
    if (t_fiber)
    {
        return t_fiber->getId();
    }
    return 0;
}

void Fiber::MainFunc()
{
    Fiber::ptr cur = GetThis();
    PHASE0_ASSERT(cur);
    try
    {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    }
    catch (std::exception& ex)
    {
        cur->m_state = EXCEPT;
        P0ROOT_LOG_ERROR() << "Fiber Except: " << ex.what() << " fiber id=" << cur->getId() << std::endl
                           << phase0::BacktraceToString();
    }
    catch (...)
    {
        cur->m_state = EXCEPT;
        P0ROOT_LOG_ERROR() << "Fiber Except"
                           << " fiber id=" << cur->getId() << std::endl
                           << phase0::BacktraceToString();
    }

    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->swapOut();

    PHASE0_ASSERT2(false, "never reach fiber id=" + std::to_string(raw_ptr->getId()));
}

void Fiber::CallerMainFunc()
{
    Fiber::ptr cur = GetThis();
    PHASE0_ASSERT(cur);
    try
    {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    }
    catch (std::exception& ex)
    {
        cur->m_state = EXCEPT;
        P0ROOT_LOG_ERROR() << "Fiber Except: " << ex.what() << " fiber id=" << cur->getId() << std::endl
                           << phase0::BacktraceToString();
    }
    catch (...)
    {
        cur->m_state = EXCEPT;
        P0ROOT_LOG_ERROR() << "Fiber Except"
                           << " fiber id=" << cur->getId() << std::endl
                           << phase0::BacktraceToString();
    }

    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->back();
    PHASE0_ASSERT2(false, "never reach fiber id=" + std::to_string(raw_ptr->getId()));
}

}  // namespace phase0