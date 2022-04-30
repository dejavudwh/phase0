#include "HttpServer.h"

#include "LogMarco.h"

namespace phase0
{
namespace http
{
HttpServer::HttpServer(bool keepalive,
                       phase0::IOManager* worker,
                       phase0::IOManager* ioWorker,
                       phase0::IOManager* acceptWorker)
    : TcpServer(ioWorker, acceptWorker), m_isKeepalive(keepalive)
{
    m_dispatch.reset(new ServletDispatch);

    m_type = "http";
}

void HttpServer::setName(const std::string& v)
{
    TcpServer::setName(v);
    m_dispatch->setDefault(std::make_shared<NotFoundServlet>(v));
}

void HttpServer::handleClient(Socket::ptr client)
{
    P0SYS_LOG_DEBUG() << "handleClient " << *client;
    HttpSession::ptr session(new HttpSession(client));
    do
    {
        auto req = session->recvRequest();
        if (!req)
        {
            P0SYS_LOG_DEBUG() << "recv http request fail, errno=" << errno << " errstr=" << strerror(errno)
                              << " cliet:" << *client << " keep_alive=" << m_isKeepalive;
            break;
        }

        HttpResponse::ptr rsp(new HttpResponse(req->getVersion(), req->isClose() || !m_isKeepalive));
        rsp->setHeader("Server", getName());
        m_dispatch->handle(req, rsp, session);
        session->sendResponse(rsp);

        if (!m_isKeepalive || req->isClose())
        {
            break;
        }
    } while (true);
    session->close();
}

}  // namespace http
}  // namespace phase0
