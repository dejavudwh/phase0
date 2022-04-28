#include "hook.h"

#include <dlfcn.h>

#include "Config.hpp"
#include "ConfigVar.hpp"
#include "FdManager.h"
#include "LogMarco.h"
#include "fiber.h"
#include "iomanager.h"

namespace phase0
{
static phase0::ConfigVar<int>::ptr DefaultTcpConnectTimeout =
    phase0::Config::lookup("tcp.connect.timeout", 5000, "tcp connect timeout");

static thread_local bool t_hookEnable = false;

#define HOOK_FUN(XX) \
    XX(sleep)        \
    XX(usleep)       \
    XX(nanosleep)    \
    XX(socket)       \
    XX(connect)      \
    XX(accept)       \
    XX(read)         \
    XX(readv)        \
    XX(recv)         \
    XX(recvfrom)     \
    XX(recvmsg)      \
    XX(write)        \
    XX(writev)       \
    XX(send)         \
    XX(sendto)       \
    XX(sendmsg)      \
    XX(close)        \
    XX(fcntl)        \
    XX(ioctl)        \
    XX(getsockopt)   \
    XX(setsockopt)

void hookInit()
{
    static bool isInited = false;
    if (isInited)
    {
        return;
    }
#define XX(name) name##_f = (name##_fun)dlsym(RTLD_NEXT, #name);
    HOOK_FUN(XX);
#undef XX
}

static uint64_t ConnectTimeout = -1;
struct _HookIniter
{
    _HookIniter()
    {
        hookInit();
        ConnectTimeout = DefaultTcpConnectTimeout->getValue();

        DefaultTcpConnectTimeout->addListener([](const int& old_value, const int& new_value) {
            P0SYS_LOG_INFO() << "tcp connect timeout changed from " << old_value << " to " << new_value;
            ConnectTimeout = new_value;
        });
    }
};

static _HookIniter HookIniter;

bool isHookEnable() { return t_hookEnable; }

void setHookEnable(bool flag) { t_hookEnable = flag; }

}  // namespace phase0

struct TimerInfo
{
    int cancelled = 0;
};

template <typename OriginFun, typename... Args>
static ssize_t do_io(
    int fd, OriginFun fun, const char* hookFunName, uint32_t event, int timeoutSo, Args&&... args)
{
    if (!phase0::t_hookEnable)
    {
        return fun(fd, std::forward<Args>(args)...);
    }

    phase0::FdCtx::ptr ctx = phase0::FdManagerInstanse::GetInstance()->get(fd);
    if (!ctx)
    {
        return fun(fd, std::forward<Args>(args)...);
    }

    if (ctx->isClose())
    {
        errno = EBADF;
        return -1;
    }

    if (!ctx->isSocket() || ctx->getUserNonblock())
    {
        return fun(fd, std::forward<Args>(args)...);
    }

    uint64_t to = ctx->getTimeout(timeoutSo);
    std::shared_ptr<TimerInfo> tinfo(new TimerInfo);

retry:
    ssize_t n = fun(fd, std::forward<Args>(args)...);
    while (n == -1 && errno == EINTR)
    {
        n = fun(fd, std::forward<Args>(args)...);
    }
    if (n == -1 && errno == EAGAIN)
    {
        phase0::IOManager* iom = phase0::IOManager::GetThis();
        phase0::Timer::ptr timer;
        std::weak_ptr<TimerInfo> winfo(tinfo);

        if (to != (uint64_t)-1)
        {
            timer = iom->addConditionTimer(
                to,
                [winfo, fd, iom, event]() {
                    auto t = winfo.lock();
                    if (!t || t->cancelled)
                    {
                        return;
                    }
                    t->cancelled = ETIMEDOUT;
                    iom->cancelEvent(fd, (phase0::IOManager::Event)(event));
                },
                winfo);
        }

        int rt = iom->addEvent(fd, (phase0::IOManager::Event)(event));
        if (PHASE0_UNLIKELY(rt))
        {
            P0SYS_LOG_ERROR() << hookFunName << " addEvent(" << fd << ", " << event << ")";
            if (timer)
            {
                timer->cancel();
            }
            return -1;
        }
        else
        {
            phase0::Fiber::GetThis()->YieldToReady();
            if (timer)
            {
                timer->cancel();
            }
            if (tinfo->cancelled)
            {
                errno = tinfo->cancelled;
                return -1;
            }
            goto retry;
        }
    }

    return n;
}

extern "C" {
#define XX(name) name##_fun name##_f = nullptr;
HOOK_FUN(XX);
#undef XX

unsigned int sleep(unsigned int seconds)
{
    if (!phase0::t_hookEnable)
    {
        return sleep_f(seconds);
    }

    phase0::Fiber::ptr fiber = phase0::Fiber::GetThis();
    phase0::IOManager* iom = phase0::IOManager::GetThis();
    iom->addTimer(
        seconds * 1000,
        std::bind((void (phase0::Scheduler::*)(phase0::Fiber::ptr, int thread)) & phase0::IOManager::schedule,
                  iom,
                  fiber,
                  -1));
    phase0::Fiber::GetThis()->YieldToReady();
    return 0;
}

int usleep(useconds_t usec)
{
    if (!phase0::t_hookEnable)
    {
        return usleep_f(usec);
    }
    phase0::Fiber::ptr fiber = phase0::Fiber::GetThis();
    phase0::IOManager* iom = phase0::IOManager::GetThis();
    iom->addTimer(
        usec / 1000,
        std::bind((void (phase0::Scheduler::*)(phase0::Fiber::ptr, int thread)) & phase0::IOManager::schedule,
                  iom,
                  fiber,
                  -1));
    phase0::Fiber::GetThis()->YieldToReady();
    return 0;
}

int nanosleep(const struct timespec* req, struct timespec* rem)
{
    if (!phase0::t_hookEnable)
    {
        return nanosleep_f(req, rem);
    }

    int timeout_ms = req->tv_sec * 1000 + req->tv_nsec / 1000 / 1000;
    phase0::Fiber::ptr fiber = phase0::Fiber::GetThis();
    phase0::IOManager* iom = phase0::IOManager::GetThis();
    iom->addTimer(
        timeout_ms,
        std::bind((void (phase0::Scheduler::*)(phase0::Fiber::ptr, int thread)) & phase0::IOManager::schedule,
                  iom,
                  fiber,
                  -1));
    phase0::Fiber::GetThis()->YieldToReady();
    return 0;
}

int socket(int domain, int type, int protocol)
{
    if (!phase0::t_hookEnable)
    {
        return socket_f(domain, type, protocol);
    }
    int fd = socket_f(domain, type, protocol);
    if (fd == -1)
    {
        return fd;
    }
    phase0::FdManagerInstanse::GetInstance()->get(fd, true);
    return fd;
}

int connect_with_timeout(int fd, const struct sockaddr* addr, socklen_t addrlen, uint64_t timeoutMs)
{
    if (!phase0::t_hookEnable)
    {
        return connect_f(fd, addr, addrlen);
    }
    phase0::FdCtx::ptr ctx = phase0::FdManagerInstanse::GetInstance()->get(fd);
    if (!ctx || ctx->isClose())
    {
        errno = EBADF;
        return -1;
    }

    if (!ctx->isSocket())
    {
        return connect_f(fd, addr, addrlen);
    }

    if (ctx->getUserNonblock())
    {
        return connect_f(fd, addr, addrlen);
    }

    int n = connect_f(fd, addr, addrlen);
    if (n == 0)
    {
        return 0;
    }
    else if (n != -1 || errno != EINPROGRESS)
    {
        return n;
    }

    phase0::IOManager* iom = phase0::IOManager::GetThis();
    phase0::Timer::ptr timer;
    std::shared_ptr<TimerInfo> tinfo(new TimerInfo);
    std::weak_ptr<TimerInfo> winfo(tinfo);

    if (timeoutMs != (uint64_t)-1)
    {
        timer = iom->addConditionTimer(
            timeoutMs,
            [winfo, fd, iom]() {
                auto t = winfo.lock();
                if (!t || t->cancelled)
                {
                    return;
                }
                t->cancelled = ETIMEDOUT;
                iom->cancelEvent(fd, phase0::IOManager::WRITE);
            },
            winfo);
    }

    int rt = iom->addEvent(fd, phase0::IOManager::WRITE);
    if (rt == 0)
    {
        phase0::Fiber::GetThis()->YieldToReady();
        if (timer)
        {
            timer->cancel();
        }
        if (tinfo->cancelled)
        {
            errno = tinfo->cancelled;
            return -1;
        }
    }
    else
    {
        if (timer)
        {
            timer->cancel();
        }
        P0SYS_LOG_ERROR() << "connect addEvent(" << fd << ", WRITE) error";
    }

    int error = 0;
    socklen_t len = sizeof(int);
    if (-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len))
    {
        return -1;
    }
    if (!error)
    {
        return 0;
    }
    else
    {
        errno = error;
        return -1;
    }
}

int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
    return connect_with_timeout(sockfd, addr, addrlen, phase0::ConnectTimeout);
}

int accept(int s, struct sockaddr* addr, socklen_t* addrlen)
{
    int fd = do_io(s, accept_f, "accept", phase0::IOManager::READ, SO_RCVTIMEO, addr, addrlen);
    if (fd >= 0)
    {
        phase0::FdManagerInstanse::GetInstance()->get(fd, true);
    }
    return fd;
}

ssize_t read(int fd, void* buf, size_t count)
{
    return do_io(fd, read_f, "read", phase0::IOManager::READ, SO_RCVTIMEO, buf, count);
}

ssize_t readv(int fd, const struct iovec* iov, int iovcnt)
{
    return do_io(fd, readv_f, "readv", phase0::IOManager::READ, SO_RCVTIMEO, iov, iovcnt);
}

ssize_t recv(int sockfd, void* buf, size_t len, int flags)
{
    return do_io(sockfd, recv_f, "recv", phase0::IOManager::READ, SO_RCVTIMEO, buf, len, flags);
}

ssize_t recvfrom(int sockfd, void* buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t* addrlen)
{
    return do_io(sockfd,
                 recvfrom_f,
                 "recvfrom",
                 phase0::IOManager::READ,
                 SO_RCVTIMEO,
                 buf,
                 len,
                 flags,
                 src_addr,
                 addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr* msg, int flags)
{
    return do_io(sockfd, recvmsg_f, "recvmsg", phase0::IOManager::READ, SO_RCVTIMEO, msg, flags);
}

ssize_t write(int fd, const void* buf, size_t count)
{
    return do_io(fd, write_f, "write", phase0::IOManager::WRITE, SO_SNDTIMEO, buf, count);
}

ssize_t writev(int fd, const struct iovec* iov, int iovcnt)
{
    return do_io(fd, writev_f, "writev", phase0::IOManager::WRITE, SO_SNDTIMEO, iov, iovcnt);
}

ssize_t send(int s, const void* msg, size_t len, int flags)
{
    return do_io(s, send_f, "send", phase0::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags);
}

ssize_t sendto(int s, const void* msg, size_t len, int flags, const struct sockaddr* to, socklen_t tolen)
{
    return do_io(s, sendto_f, "sendto", phase0::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags, to, tolen);
}

ssize_t sendmsg(int s, const struct msghdr* msg, int flags)
{
    return do_io(s, sendmsg_f, "sendmsg", phase0::IOManager::WRITE, SO_SNDTIMEO, msg, flags);
}

int close(int fd)
{
    if (!phase0::t_hookEnable)
    {
        return close_f(fd);
    }

    phase0::FdCtx::ptr ctx = phase0::FdManagerInstanse::GetInstance()->get(fd);
    if (ctx)
    {
        auto iom = phase0::IOManager::GetThis();
        if (iom)
        {
            iom->cancelAll(fd);
        }
        phase0::FdManagerInstanse::GetInstance()->del(fd);
    }
    return close_f(fd);
}

int fcntl(int fd, int cmd, ... /* arg */)
{
    va_list va;
    va_start(va, cmd);
    switch (cmd)
    {
        case F_SETFL: {
            int arg = va_arg(va, int);
            va_end(va);
            phase0::FdCtx::ptr ctx = phase0::FdManagerInstanse::GetInstance()->get(fd);
            if (!ctx || ctx->isClose() || !ctx->isSocket())
            {
                return fcntl_f(fd, cmd, arg);
            }
            ctx->setUserNonblock(arg & O_NONBLOCK);
            if (ctx->getSysNonblock())
            {
                arg |= O_NONBLOCK;
            }
            else
            {
                arg &= ~O_NONBLOCK;
            }
            return fcntl_f(fd, cmd, arg);
        }
        break;
        case F_GETFL: {
            va_end(va);
            int arg = fcntl_f(fd, cmd);
            phase0::FdCtx::ptr ctx = phase0::FdManagerInstanse::GetInstance()->get(fd);
            if (!ctx || ctx->isClose() || !ctx->isSocket())
            {
                return arg;
            }
            if (ctx->getUserNonblock())
            {
                return arg | O_NONBLOCK;
            }
            else
            {
                return arg & ~O_NONBLOCK;
            }
        }
        break;
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_SETFD:
        case F_SETOWN:
        case F_SETSIG:
        case F_SETLEASE:
        case F_NOTIFY:
#ifdef F_SETPIPE_SZ
        case F_SETPIPE_SZ:
#endif
        {
            int arg = va_arg(va, int);
            va_end(va);
            return fcntl_f(fd, cmd, arg);
        }
        break;
        case F_GETFD:
        case F_GETOWN:
        case F_GETSIG:
        case F_GETLEASE:
#ifdef F_GETPIPE_SZ
        case F_GETPIPE_SZ:
#endif
        {
            va_end(va);
            return fcntl_f(fd, cmd);
        }
        break;
        case F_SETLK:
        case F_SETLKW:
        case F_GETLK: {
            struct flock* arg = va_arg(va, struct flock*);
            va_end(va);
            return fcntl_f(fd, cmd, arg);
        }
        break;
        case F_GETOWN_EX:
        case F_SETOWN_EX: {
            struct f_owner_exlock* arg = va_arg(va, struct f_owner_exlock*);
            va_end(va);
            return fcntl_f(fd, cmd, arg);
        }
        break;
        default:
            va_end(va);
            return fcntl_f(fd, cmd);
    }
}

int ioctl(int d, unsigned long int request, ...)
{
    va_list va;
    va_start(va, request);
    void* arg = va_arg(va, void*);
    va_end(va);

    if (FIONBIO == request)
    {
        bool userNonblock = !!*(int*)arg;
        phase0::FdCtx::ptr ctx = phase0::FdManagerInstanse::GetInstance()->get(d);
        if (!ctx || ctx->isClose() || !ctx->isSocket())
        {
            return ioctl_f(d, request, arg);
        }
        ctx->setUserNonblock(userNonblock);
    }
    return ioctl_f(d, request, arg);
}

int getsockopt(int sockfd, int level, int optname, void* optval, socklen_t* optlen)
{
    return getsockopt_f(sockfd, level, optname, optval, optlen);
}

int setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen)
{
    if (!phase0::t_hookEnable)
    {
        return setsockopt_f(sockfd, level, optname, optval, optlen);
    }
    if (level == SOL_SOCKET)
    {
        if (optname == SO_RCVTIMEO || optname == SO_SNDTIMEO)
        {
            phase0::FdCtx::ptr ctx = phase0::FdManagerInstanse::GetInstance()->get(sockfd);
            if (ctx)
            {
                const timeval* v = (const timeval*)optval;
                ctx->setTimeout(optname, v->tv_sec * 1000 + v->tv_usec / 1000);
            }
        }
    }
    return setsockopt_f(sockfd, level, optname, optval, optlen);
}
}
