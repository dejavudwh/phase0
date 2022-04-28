#pragma once

#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <memory>

#include "LogMarco.h"
#include "scheduler.h"

namespace phase0
{
class IOManager : public Scheduler
{
public:
    using ptr = std::shared_ptr<IOManager>;

    enum Event
    {
        NONE = 0x0,
        READ = 0x1,
        WRITE = 0x4,
    };

private:
    struct FdContext
    {
        struct EventContext
        {
            Scheduler *scheduler = nullptr;
            Fiber::ptr fiber;
            std::function<void()> cb;
        };

        EventContext &getEventContext(Event event);
        void resetEventContext(EventContext &ctx);
        void triggerEvent(Event event);

        EventContext read;
        EventContext write;
        int fd = 0;
        Event events = NONE;
        std::mutex mutex;
    };

public:
    IOManager(size_t threads = 1, bool useCaller = true, const std::string &name = "IOManager");
    ~IOManager();

    int addEvent(int fd, Event event, std::function<void()> cb = nullptr);
    bool delEvent(int fd, Event event);
    bool cancelEvent(int fd, Event event);
    bool cancelAll(int fd);

    static IOManager *GetThis();

protected:
    void tickle() override;
    bool stopping() override;
    void idle() override;

    void contextResize(size_t size);

private:
    int m_epfd = 0;
    int m_tickleFds[2];
    std::atomic<size_t> m_pendingEventCount = {0};
    std::mutex m_mutex;
    std::vector<FdContext *> m_fdContexts;
};

}  // namespace phase0