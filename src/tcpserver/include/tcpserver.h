#pragma once

#include <functional>
#include <memory>

#include "Config.hpp"
#include "address.h"
#include "iomanager.h"
#include "noncopyable.h"
#include "socket.h"

namespace phase0
{
class TcpServer : public std::enable_shared_from_this<TcpServer>, Noncopyable
{
public:
    using ptr = std::shared_ptr<TcpServer>;

    TcpServer(phase0::IOManager* ioWoker = phase0::IOManager::GetThis(),
              phase0::IOManager* acceptWorker = phase0::IOManager::GetThis());

    virtual ~TcpServer();

    virtual bool bind(phase0::Address::ptr addr);
    virtual bool bind(const std::vector<Address::ptr>& addrs, std::vector<Address::ptr>& fails);

    virtual bool start();
    virtual void stop();

    uint64_t getRecvTimeout() const { return m_recvTimeout; }
    std::string getName() const { return m_name; }

    void setRecvTimeout(uint64_t v) { m_recvTimeout = v; }
    virtual void setName(const std::string& v) { m_name = v; }

    bool isStop() const { return m_isStop; }

    virtual std::string toString(const std::string& prefix = "");

protected:
    virtual void handleClient(Socket::ptr client);
    virtual void startAccept(Socket::ptr sock);

protected:
    std::vector<Socket::ptr> m_socks;
    IOManager* m_ioWorker;
    IOManager* m_acceptWorker;
    uint64_t m_recvTimeout;
    std::string m_name;
    std::string m_type;
    bool m_isStop;
};

}  // namespace phase0
