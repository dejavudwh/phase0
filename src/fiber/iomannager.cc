#include <cstring>
#include <memory>
#include <mutex>

#include "iomanager.h"

namespace phase0
{
enum EpollCtlOp
{
};

static std::ostream& operator<<(std::ostream& os, const EpollCtlOp& op)
{
    switch ((int)op)
    {
#define XX(ctl) \
    case ctl:   \
        return os << #ctl;
        XX(EPOLL_CTL_ADD);
        XX(EPOLL_CTL_MOD);
        XX(EPOLL_CTL_DEL);
#undef XX
        default:
            return os << (int)op;
    }
}

static std::ostream& operator<<(std::ostream& os, EPOLL_EVENTS events)
{
    if (!events)
    {
        return os << "0";
    }
    bool first = true;
#define XX(E)          \
    if (events & E)    \
    {                  \
        if (!first)    \
        {              \
            os << "|"; \
        }              \
        os << #E;      \
        first = false; \
    }
    XX(EPOLLIN);
    XX(EPOLLPRI);
    XX(EPOLLOUT);
    XX(EPOLLRDNORM);
    XX(EPOLLRDBAND);
    XX(EPOLLWRNORM);
    XX(EPOLLWRBAND);
    XX(EPOLLMSG);
    XX(EPOLLERR);
    XX(EPOLLHUP);
    XX(EPOLLRDHUP);
    XX(EPOLLONESHOT);
    XX(EPOLLET);
#undef XX
    return os;
}

IOManager::FdContext::EventContext& IOManager::FdContext::getEventContext(IOManager::Event event)
{
    switch (event)
    {
        case IOManager::READ:
            return read;
        case IOManager::WRITE:
            return write;
        default:
            PHASE0_ASSERT2(false, "Get FdContext");
    }
    throw std::invalid_argument("Get FdContext invalid event");
}

void IOManager::FdContext::resetEventContext(EventContext& ctx)
{
    ctx.scheduler = nullptr;
    ctx.fiber.reset();
    ctx.cb = nullptr;
}

void IOManager::FdContext::triggerEvent(IOManager::Event event)
{
    PHASE0_ASSERT(events & event);

    events = (Event)(events & ~event);

    EventContext& ctx = getEventContext(event);
    if (ctx.cb)
    {
        ctx.scheduler->schedule(ctx.cb);
    }
    else
    {
        ctx.scheduler->schedule(ctx.fiber);
    }
    resetEventContext(ctx);
    return;
}

IOManager::IOManager(size_t threads, bool useCaller, const std::string& name)
    : Scheduler(threads, useCaller, name)
{
    m_epfd = epoll_create(5000);
    PHASE0_ASSERT(m_epfd > 0);

    int rt = pipe(m_tickleFds);
    PHASE0_ASSERT(!rt);

    epoll_event event;
    memset(&event, 0, sizeof(epoll_event));
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = m_tickleFds[0];

    rt = fcntl(m_tickleFds[0], F_SETFL, O_NONBLOCK);
    PHASE0_ASSERT(!rt);

    rt = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickleFds[0], &event);
    PHASE0_ASSERT(!rt);

    contextResize(32);

    start();
}

IOManager::~IOManager()
{
    stop();
    close(m_epfd);
    close(m_tickleFds[0]);
    close(m_tickleFds[1]);

    for (size_t i = 0; i < m_fdContexts.size(); ++i)
    {
        if (m_fdContexts[i])
        {
            delete m_fdContexts[i];
        }
    }
}

void IOManager::contextResize(size_t size)
{
    m_fdContexts.resize(size);

    for (size_t i = 0; i < m_fdContexts.size(); ++i)
    {
        if (!m_fdContexts[i])
        {
            m_fdContexts[i] = new FdContext();
            m_fdContexts[i]->fd = i;
        }
    }
}

int IOManager::addEvent(int fd, Event event, std::function<void()> cb)
{
    FdContext* fdCtx = nullptr;
    std::unique_lock<std::mutex> lock(m_mutex);
    if ((int)m_fdContexts.size() > fd)
    {
        fdCtx = m_fdContexts[fd];
        lock.unlock();
    }
    else
    {
        lock.unlock();
        std::unique_lock<std::mutex> lock2(m_mutex);
        contextResize(fd * 1.5);
        fdCtx = m_fdContexts[fd];
    }

    std::unique_lock<std::mutex> lock2(fdCtx->mutex);
    if (PHASE0_UNLIKELY(fdCtx->events & event))
    {
        P0SYS_LOG_ERROR() << "addEvent assert fd=" << fd << " event=" << (EPOLL_EVENTS)event
                          << " fd_ctx.event=" << (EPOLL_EVENTS)fdCtx->events;
        PHASE0_ASSERT(!(fdCtx->events & event));
    }

    int op = fdCtx->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    epoll_event epevent;
    epevent.events = EPOLLET | fdCtx->events | event;
    epevent.data.ptr = fdCtx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if (rt)
    {
        P0SYS_LOG_ERROR() << "epoll_ctl(" << m_epfd << ", " << (EpollCtlOp)op << ", " << fd << ", "
                          << (EPOLL_EVENTS)epevent.events << "):" << rt << " (" << errno << ") ("
                          << strerror(errno) << ") fd_ctx->events=" << (EPOLL_EVENTS)fdCtx->events;
        return -1;
    }

    ++m_pendingEventCount;

    fdCtx->events = (Event)(fdCtx->events | event);
    FdContext::EventContext& eventCtx = fdCtx->getEventContext(event);
    PHASE0_ASSERT(!eventCtx.scheduler && !eventCtx.fiber && !eventCtx.cb);

    eventCtx.scheduler = Scheduler::GetThis();
    if (cb)
    {
        eventCtx.cb.swap(cb);
    }
    else
    {
        eventCtx.fiber = Fiber::GetThis();
        PHASE0_ASSERT2(eventCtx.fiber->getState() == Fiber::EXEC, "Fiber state not EXEC");
    }
    return 0;
}

bool IOManager::delEvent(int fd, Event event)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if ((int)m_fdContexts.size() <= fd)
    {
        return false;
    }
    FdContext* fdCtx = m_fdContexts[fd];
    lock.unlock();

    std::unique_lock<std::mutex> lock2(fdCtx->mutex);
    if (PHASE0_UNLIKELY(!(fdCtx->events & event)))
    {
        return false;
    }

    Event newEvents = (Event)(fdCtx->events & ~event);
    int op = newEvents ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = EPOLLET | newEvents;
    epevent.data.ptr = fdCtx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if (rt)
    {
        P0SYS_LOG_ERROR() << "epoll_ctl(" << m_epfd << ", " << (EpollCtlOp)op << ", " << fd << ", "
                          << (EPOLL_EVENTS)epevent.events << "):" << rt << " (" << errno << ") ("
                          << strerror(errno) << ")";
        return false;
    }

    --m_pendingEventCount;

    fdCtx->events = newEvents;
    FdContext::EventContext& eventCtx = fdCtx->getEventContext(event);
    fdCtx->resetEventContext(eventCtx);
    return true;
}

bool IOManager::cancelEvent(int fd, Event event)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if ((int)m_fdContexts.size() <= fd)
    {
        return false;
    }
    FdContext* fdCtx = m_fdContexts[fd];
    lock.unlock();

    std::unique_lock<std::mutex> lock2(fdCtx->mutex);
    if (PHASE0_UNLIKELY(!(fdCtx->events & event)))
    {
        return false;
    }

    Event newEvents = (Event)(fdCtx->events & ~event);
    int op = newEvents ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = EPOLLET | newEvents;
    epevent.data.ptr = fdCtx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if (rt)
    {
        P0SYS_LOG_ERROR() << "epoll_ctl(" << m_epfd << ", " << (EpollCtlOp)op << ", " << fd << ", "
                          << (EPOLL_EVENTS)epevent.events << "):" << rt << " (" << errno << ") ("
                          << strerror(errno) << ")";
        return false;
    }

    fdCtx->triggerEvent(event);
    --m_pendingEventCount;
    return true;
}

bool IOManager::cancelAll(int fd)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if ((int)m_fdContexts.size() <= fd)
    {
        return false;
    }
    FdContext* fdCtx = m_fdContexts[fd];
    lock.unlock();

    std::unique_lock<std::mutex> lock2(fdCtx->mutex);
    if (!fdCtx->events)
    {
        return false;
    }

    int op = EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = 0;
    epevent.data.ptr = fdCtx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if (rt)
    {
        P0SYS_LOG_ERROR() << "epoll_ctl(" << m_epfd << ", " << (EpollCtlOp)op << ", " << fd << ", "
                          << (EPOLL_EVENTS)epevent.events << "):" << rt << " (" << errno << ") ("
                          << strerror(errno) << ")";
        return false;
    }

    if (fdCtx->events & READ)
    {
        fdCtx->triggerEvent(READ);
        --m_pendingEventCount;
    }
    if (fdCtx->events & WRITE)
    {
        fdCtx->triggerEvent(WRITE);
        --m_pendingEventCount;
    }

    PHASE0_ASSERT(fdCtx->events == 0);
    return true;
}

IOManager* IOManager::GetThis() { return dynamic_cast<IOManager*>(Scheduler::GetThis()); }

void IOManager::tickle()
{
    P0SYS_LOG_DEBUG() << "tickle";
    if (!hasIdleThreads())
    {
        return;
    }
    int rt = write(m_tickleFds[1], "T", 1);
    PHASE0_ASSERT(rt == 1);
}

bool IOManager::stopping() { return m_pendingEventCount == 0 && Scheduler::stopping(); }

void IOManager::idle()
{
    P0SYS_LOG_DEBUG() << "Into idle fiber";

    const uint64_t MAX_EVNETS = 256;
    epoll_event* events = new epoll_event[MAX_EVNETS]();
    std::shared_ptr<epoll_event> sharedEvents(events, [](epoll_event* ptr) { delete[] ptr; });

    while (true)
    {
        if (stopping())
        {
            P0SYS_LOG_DEBUG() << "Fiber: name=" << getName() << "idle stopping exit";
            break;
        }

        static const int MAX_TIMEOUT = 5000;
        int rt = epoll_wait(m_epfd, events, MAX_EVNETS, MAX_TIMEOUT);
        if (rt < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            P0SYS_LOG_ERROR() << "epoll_wait(" << m_epfd << ") (rt=" << rt << ") (errno=" << errno
                              << ") (errstr:" << strerror(errno) << ")";
            break;
        }

        for (int i = 0; i < rt; ++i)
        {
            epoll_event& event = events[i];
            if (event.data.fd == m_tickleFds[0])
            {
                uint8_t dummy[256];
                while (read(m_tickleFds[0], dummy, sizeof(dummy)) > 0)
                    ;
                continue;
            }

            FdContext* fdCtx = (FdContext*)event.data.ptr;
            std::unique_lock<std::mutex> lock(fdCtx->mutex);

            if (event.events & (EPOLLERR | EPOLLHUP))
            {
                event.events |= (EPOLLIN | EPOLLOUT) & fdCtx->events;
            }

            int realEvents = NONE;
            if (event.events & EPOLLIN)
            {
                realEvents |= READ;
            }
            if (event.events & EPOLLOUT)
            {
                realEvents |= WRITE;
            }

            if ((fdCtx->events & realEvents) == NONE)
            {
                continue;
            }

            int leftEvents = (fdCtx->events & ~realEvents);
            int op = leftEvents ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
            event.events = EPOLLET | leftEvents;

            int rt2 = epoll_ctl(m_epfd, op, fdCtx->fd, &event);
            if (rt2)
            {
                P0SYS_LOG_ERROR() << "epoll_ctl(" << m_epfd << ", " << (EpollCtlOp)op << ", " << fdCtx->fd
                                  << ", " << (EPOLL_EVENTS)event.events << "):" << rt2 << " (" << errno
                                  << ") (" << strerror(errno) << ")";
                continue;
            }

            if (realEvents & READ)
            {
                fdCtx->triggerEvent(READ);
                --m_pendingEventCount;
            }
            if (realEvents & WRITE)
            {
                fdCtx->triggerEvent(WRITE);
                --m_pendingEventCount;
            }
        }  // end for

        Fiber::ptr cur = Fiber::GetThis();
        auto raw_ptr = cur.get();
        cur.reset();

        raw_ptr->YieldToReady();
    }  // end while(true)
}

}  // namespace phase0