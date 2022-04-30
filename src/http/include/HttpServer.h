#pragma once

#include "HttpSession.h"
#include "servlet.h"
#include "tcpserver.h"

namespace phase0
{
namespace http
{
/**
 * @brief HTTP服务器类
 */
class HttpServer : public TcpServer
{
public:
    using ptr = std::shared_ptr<HttpServer>;

    HttpServer(bool keepalive = false,
               phase0::IOManager* worker = phase0::IOManager::GetThis(),
               phase0::IOManager* ioWorker = phase0::IOManager::GetThis(),
               phase0::IOManager* acceptWorker = phase0::IOManager::GetThis());


    ServletDispatch::ptr getServletDispatch() const { return m_dispatch; }
    void setServletDispatch(ServletDispatch::ptr v) { m_dispatch = v; }

    virtual void setName(const std::string& v) override;

protected:
    virtual void handleClient(Socket::ptr client) override;

private:
    bool m_isKeepalive;
    ServletDispatch::ptr m_dispatch;
};

}  // namespace http
}  // namespace phase0
