#include "scheduler.h"

#include <mutex>

#include "LogMarco.h"
#include "hook.h"
#include "utils.h"

namespace phase0
{
static thread_local Scheduler* t_scheduler = nullptr;
static thread_local Fiber* t_schedulerFiber = nullptr;

Scheduler::Scheduler(size_t threads, bool useCaller, const std::string& name)
{
    PHASE0_ASSERT(threads > 0);

    m_useCaller = useCaller;
    m_name = name;

    if (useCaller)
    {
        --threads;
        Fiber::GetThis();
        PHASE0_ASSERT(GetThis() == nullptr);
        t_scheduler = this;

        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, false));

        phase0::SetCurThreadName(m_name);
        t_schedulerFiber = m_rootFiber.get();
        m_rootThread = phase0::GetCurThreadId();
        m_threadIds.push_back(m_rootThread);
    }
    else
    {
        m_rootThread = -1;
    }
    m_threadCount = threads;
}

Scheduler::~Scheduler()
{
    P0SYS_LOG_DEBUG() << "Scheduler::~Scheduler()";
    PHASE0_ASSERT(m_stopping);
    if (GetThis() == this)
    {
        t_scheduler = nullptr;
    }
}

Scheduler* Scheduler::GetThis() { return t_scheduler; }

Fiber* Scheduler::GetMainFiber() { return t_schedulerFiber; }

void Scheduler::setThis() { t_scheduler = this; }

void Scheduler::start()
{
    P0SYS_LOG_DEBUG() << "Scheduler start!";
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_stopping)
    {
        P0SYS_LOG_DEBUG() << "Scheduler is stopped";
        return;
    }
    PHASE0_ASSERT(m_threads.empty());
    m_threads.resize(m_threadCount);
    for (size_t i = 0; i < m_threadCount; i++)
    {
        std::thread t1(std::bind(&Scheduler::run, this));
        SetThreadName(t1, m_name + "_" + std::to_string(i));
        
        m_threads[i].swap(t1);
        m_threadIds.push_back(GetThreadId(m_threads[i]));
    }
}

void Scheduler::run()
{
    P0SYS_LOG_DEBUG() << "Scheduler run!";
    setThis();
    setHookEnable(true);
    if (phase0::GetCurThreadId() != m_rootThread)
    {
        t_schedulerFiber = Fiber::GetThis().get();
    }

    Fiber::ptr idleFiber(new Fiber(std::bind(&Scheduler::idle, this)));
    Fiber::ptr cbFiber;

    ScheduleTask task;
    while (true)
    {
        task.reset();
        bool tickleMe = false;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            auto it = m_tasks.begin();
            while (it != m_tasks.end())
            {
                if (it->thread != -1 && it->thread != phase0::GetCurThreadId())
                {
                    ++it;
                    tickleMe = true;
                    continue;
                }

                PHASE0_ASSERT(it->fiber || it->cb);
                if (it->fiber)
                {
                    PHASE0_ASSERT(it->fiber->getState() == Fiber::READY);
                }

                task = *it;
                m_tasks.erase(it++);
                ++m_activeThreadCount;
                break;
            }

            tickleMe |= (it != m_tasks.end());
        }

        if (tickleMe)
        {
            tickle();
        }

        if (task.fiber)
        {
            task.fiber->swapIn();
            --m_activeThreadCount;
            task.reset();
        }
        else if (task.cb)
        {
            if (cbFiber)
            {
                cbFiber->reset(task.cb);
            }
            else
            {
                cbFiber.reset(new Fiber(task.cb));
            }
            task.reset();
            cbFiber->swapIn();
            --m_activeThreadCount;
            cbFiber.reset();
        }
        else
        {
            if (idleFiber->getState() == Fiber::TERM)
            {
                P0SYS_LOG_DEBUG() << "Idle fiber term";
                break;
            }
            ++m_idleThreadCount;
            idleFiber->swapIn();
            --m_idleThreadCount;
        }
    }
    P0SYS_LOG_DEBUG() << "Scheduler::run() exit";
}

void Scheduler::stop()
{
    P0SYS_LOG_DEBUG() << "Scheduler Stopping!";
    if (stopping())
    {
        return;
    }
    m_stopping = true;

    if (m_useCaller)
    {
        PHASE0_ASSERT(GetThis() == this);
    }
    else
    {
        PHASE0_ASSERT(GetThis() != this);
    }

    for (size_t i = 0; i < m_threadCount; i++)
    {
        tickle();
    }

    if (m_rootFiber)
    {
        tickle();
    }

    if (m_rootFiber)
    {
        P0SYS_LOG_DEBUG() << "root fiber in";
        m_rootFiber->swapIn();
        P0SYS_LOG_DEBUG() << "root fiber end";
    }

    std::vector<std::thread> thrs;
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        thrs.swap(m_threads);
    }
    for (auto& i : thrs)
    {
        i.join();
    }
}

bool Scheduler::stopping()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_stopping && m_tasks.empty() && m_activeThreadCount == 0;
}

void Scheduler::tickle() { P0SYS_LOG_DEBUG() << "Ticlke"; }

void Scheduler::idle()
{
    P0SYS_LOG_DEBUG() << "Into idle fiber";
    while (!stopping())
    {
        Fiber::GetThis()->YieldToReady();
    }
}

}  // namespace phase0