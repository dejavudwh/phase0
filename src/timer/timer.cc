#include "timer.h"

#include <mutex>

#include "LogMarco.h"
#include "utils.h"

namespace phase0
{
Timer::Timer(uint64_t ms, std::function<void()> cb, bool recurring, TimerManager* manager)
    : m_recurring(recurring), m_ms(ms), m_cb(cb), m_manager(manager)
{
    m_next = phase0::GetElapsedMS() + m_ms;
}

Timer::Timer(uint64_t next) : m_next(next) {}

bool Timer::cancel()
{
    std::unique_lock<std::mutex> lock(m_manager->m_mutex);
    if (m_cb)
    {
        m_cb = nullptr;
        auto it = m_manager->m_timers.find(shared_from_this());
        m_manager->m_timers.erase(it);
        return true;
    }
    return false;
}

bool Timer::refresh()
{
    std::unique_lock<std::mutex> lock(m_manager->m_mutex);
    if (!m_cb)
    {
        return false;
    }
    auto it = m_manager->m_timers.find(shared_from_this());
    if (it == m_manager->m_timers.end())
    {
        return false;
    }
    m_manager->m_timers.erase(it);
    m_next = phase0::GetElapsedMS() + m_ms;
    m_manager->m_timers.insert(shared_from_this());
    return true;
}

bool Timer::reset(uint64_t ms, bool fromNow)
{
    if (ms == m_ms && !fromNow)
    {
        return true;
    }
    std::unique_lock<std::mutex> lock(m_manager->m_mutex);
    if (!m_cb)
    {
        return false;
    }
    auto it = m_manager->m_timers.find(shared_from_this());
    if (it == m_manager->m_timers.end())
    {
        return false;
    }
    m_manager->m_timers.erase(it);
    uint64_t start = 0;
    if (fromNow)
    {
        start = phase0::GetElapsedMS();
    }
    else
    {
        start = m_next - m_ms;
    }
    m_ms = ms;
    m_next = start + m_ms;
    m_manager->addTimer(shared_from_this(), lock);
    return true;
}

bool Timer::Comparator::operator()(const Timer::ptr& lhs, const Timer::ptr& rhs) const
{
    if (!lhs && !rhs)
    {
        return false;
    }
    if (!lhs)
    {
        return true;
    }
    if (!rhs)
    {
        return false;
    }
    if (lhs->m_next < rhs->m_next)
    {
        return true;
    }
    if (rhs->m_next < lhs->m_next)
    {
        return false;
    }
    return lhs.get() < rhs.get();
}

//////////////////////////////////////////////////////////////////
// Timer Manager

TimerManager::TimerManager() { m_previouseTime = phase0::GetElapsedMS(); }

TimerManager::~TimerManager() {}

Timer::ptr TimerManager::addTimer(uint64_t ms, std::function<void()> cb, bool recurring)
{
    Timer::ptr timer(new Timer(ms, cb, recurring, this));
    std::unique_lock<std::mutex> lock(m_mutex);
    addTimer(timer, lock);
    return timer;
}

static void OnTimer(std::weak_ptr<void> weakCond, std::function<void()> cb)
{
    std::shared_ptr<void> tmp = weakCond.lock();
    if (tmp)
    {
        cb();
    }
}

Timer::ptr TimerManager::addConditionTimer(uint64_t ms,
                                           std::function<void()> cb,
                                           std::weak_ptr<void> weakCond,
                                           bool recurring)
{
    return addTimer(ms, std::bind(&OnTimer, weakCond, cb), recurring);
}

uint64_t TimerManager::getNextTimer()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_tickled = false;
    if (m_timers.empty())
    {
        return ~0ull;
    }

    const Timer::ptr& next = *m_timers.begin();
    uint64_t nowMs = phase0::GetElapsedMS();
    if (nowMs >= next->m_next)
    {
        return 0;
    }
    else
    {
        return next->m_next - nowMs;
    }
}

void TimerManager::listExpiredCb(std::vector<std::function<void()>>& cbs)
{
    uint64_t nowMs = phase0::GetElapsedMS();
    std::vector<Timer::ptr> expired;
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_timers.empty())
        {
            return;
        }
    }
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_timers.empty())
    {
        return;
    }
    bool rollover = false;
    if (PHASE0_UNLIKELY(detectClockRollover(nowMs)))
    {
        rollover = true;
    }
    if (!rollover && ((*m_timers.begin())->m_next > nowMs))
    {
        return;
    }

    Timer::ptr nowTimer(new Timer(nowMs));
    auto it = rollover ? m_timers.end() : m_timers.lower_bound(nowTimer);
    while (it != m_timers.end() && (*it)->m_next == nowMs)
    {
        ++it;
    }
    expired.insert(expired.begin(), m_timers.begin(), it);
    m_timers.erase(m_timers.begin(), it);
    cbs.reserve(expired.size());

    for (auto& timer : expired)
    {
        cbs.push_back(timer->m_cb);
        if (timer->m_recurring)
        {
            timer->m_next = nowMs + timer->m_ms;
            m_timers.insert(timer);
        }
        else
        {
            timer->m_cb = nullptr;
        }
    }
}

void TimerManager::addTimer(Timer::ptr val, std::unique_lock<std::mutex>& lock)
{
    auto it = m_timers.insert(val).first;
    bool atFront = (it == m_timers.begin()) && !m_tickled;
    if (atFront)
    {
        m_tickled = true;
    }
    lock.unlock();

    if (atFront)
    {
        onTimerInsertedAtFront();
    }
}

bool TimerManager::detectClockRollover(uint64_t nowMs)
{
    bool rollover = false;
    if (nowMs < m_previouseTime && nowMs < (m_previouseTime - 60 * 60 * 1000))
    {
        rollover = true;
    }
    m_previouseTime = nowMs;
    return rollover;
}

bool TimerManager::hasTimer()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return !m_timers.empty();
}

}  // namespace phase0