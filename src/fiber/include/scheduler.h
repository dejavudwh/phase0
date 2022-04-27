#pragma once

#include "fiber.h"

#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace phase0
{


class Scheduler
{
public:
    using ptr = std::shared_ptr<Scheduler>;

    Scheduler(size_t threads = 1, bool useCaller = true, const std::string& name = "Scheduler");
    virtual ~Scheduler();

    const std::string& getName() const { return m_name; }
    static Scheduler* GetThis();
    static Fiber* GetMainFiber();

    template <typename FiberOrCb>
    void schedule(FiberOrCb fc, int thread = -1)
    {
        bool needTickle = false;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            needTickle = scheduleNoLock(fc, thread);
        }

        if (needTickle)
        {
            tickle();
        }
    }

    void start();
    void stop();

protected:
    virtual void tickle();

    void run();

    virtual void idle();
    virtual bool stopping();

    void setThis();

private:
    struct ScheduleTask
    {
        Fiber::ptr fiber;
        std::function<void()> cb;
        int thread;

        ScheduleTask(Fiber::ptr f, int thr)
        {
            fiber = f;
            thread = thr;
        }
        ScheduleTask(Fiber::ptr* f, int thr)
        {
            fiber.swap(*f);
            thread = thr;
        }
        ScheduleTask(std::function<void()> f, int thr)
        {
            cb = f;
            thread = thr;
        }
        ScheduleTask() { thread = -1; }

        void reset()
        {
            fiber = nullptr;
            cb = nullptr;
            thread = -1;
        }
    };

private:
    template <typename FiberOrCb>
    bool scheduleNoLock(FiberOrCb fc, int thread)
    {
        bool needTickle = m_tasks.empty();
        ScheduleTask task(fc, thread);
        if (task.fiber || task.cb)
        {
            m_tasks.push_back(task);
        }
        return needTickle;
    }

private:
    std::string m_name;
    std::mutex m_mutex;
    std::vector<std::thread> m_threads;
    std::list<ScheduleTask> m_tasks;
    std::vector<uint64_t> m_threadIds;
    size_t m_threadCount = 0;
    std::atomic<size_t> m_activeThreadCount = {0};
    std::atomic<size_t> m_idleThreadCount = {0};

    bool m_useCaller;
    Fiber::ptr m_rootFiber;
    int m_rootThread = 0;

    bool m_stopping = false;
};

}  // namespace phase0