#include "servlet.h"

#include <fnmatch.h>

#include <mutex>

namespace phase0
{
namespace http
{
FunctionServlet::FunctionServlet(callback cb) : Servlet("FunctionServlet"), m_cb(cb) {}

int32_t FunctionServlet::handle(phase0::http::HttpRequest::ptr request,
                                phase0::http::HttpResponse::ptr response,
                                phase0::http::HttpSession::ptr session)
{
    return m_cb(request, response, session);
}

ServletDispatch::ServletDispatch() : Servlet("ServletDispatch")
{
    m_default.reset(new NotFoundServlet("phase0/1.0"));
}

int32_t ServletDispatch::handle(phase0::http::HttpRequest::ptr request,
                                phase0::http::HttpResponse::ptr response,
                                phase0::http::HttpSession::ptr session)
{
    auto slt = getMatchedServlet(request->getPath());
    if (slt)
    {
        slt->handle(request, response, session);
    }
    return 0;
}

void ServletDispatch::addServlet(const std::string& uri, Servlet::ptr slt)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_datas[uri] = std::make_shared<HoldServletCreator>(slt);
}

void ServletDispatch::addServletCreator(const std::string& uri, IServletCreator::ptr creator)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_datas[uri] = creator;
}

void ServletDispatch::addGlobServletCreator(const std::string& uri, IServletCreator::ptr creator)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    for (auto it = m_globs.begin(); it != m_globs.end(); ++it)
    {
        if (it->first == uri)
        {
            m_globs.erase(it);
            break;
        }
    }
    m_globs.push_back(std::make_pair(uri, creator));
}

void ServletDispatch::addServlet(const std::string& uri, FunctionServlet::callback cb)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_datas[uri] = std::make_shared<HoldServletCreator>(std::make_shared<FunctionServlet>(cb));
}

void ServletDispatch::addGlobServlet(const std::string& uri, Servlet::ptr slt)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    for (auto it = m_globs.begin(); it != m_globs.end(); ++it)
    {
        if (it->first == uri)
        {
            m_globs.erase(it);
            break;
        }
    }
    m_globs.push_back(std::make_pair(uri, std::make_shared<HoldServletCreator>(slt)));
}

void ServletDispatch::addGlobServlet(const std::string& uri, FunctionServlet::callback cb)
{
    return addGlobServlet(uri, std::make_shared<FunctionServlet>(cb));
}

void ServletDispatch::delServlet(const std::string& uri)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_datas.erase(uri);
}

void ServletDispatch::delGlobServlet(const std::string& uri)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    for (auto it = m_globs.begin(); it != m_globs.end(); ++it)
    {
        if (it->first == uri)
        {
            m_globs.erase(it);
            break;
        }
    }
}

Servlet::ptr ServletDispatch::getServlet(const std::string& uri)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_datas.find(uri);
    return it == m_datas.end() ? nullptr : it->second->get();
}

Servlet::ptr ServletDispatch::getGlobServlet(const std::string& uri)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    for (auto it = m_globs.begin(); it != m_globs.end(); ++it)
    {
        if (it->first == uri)
        {
            return it->second->get();
        }
    }
    return nullptr;
}

Servlet::ptr ServletDispatch::getMatchedServlet(const std::string& uri)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto mit = m_datas.find(uri);
    if (mit != m_datas.end())
    {
        return mit->second->get();
    }
    for (auto it = m_globs.begin(); it != m_globs.end(); ++it)
    {
        if (!fnmatch(it->first.c_str(), uri.c_str(), 0))
        {
            return it->second->get();
        }
    }
    return m_default;
}

void ServletDispatch::listAllServletCreator(std::map<std::string, IServletCreator::ptr>& infos)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    for (auto& i : m_datas)
    {
        infos[i.first] = i.second;
    }
}

void ServletDispatch::listAllGlobServletCreator(std::map<std::string, IServletCreator::ptr>& infos)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    for (auto& i : m_globs)
    {
        infos[i.first] = i.second;
    }
}

NotFoundServlet::NotFoundServlet(const std::string& name) : Servlet("NotFoundServlet"), m_name(name)
{
    m_content =
        "<html><head><title>404 Not Found"
        "</title></head><body><center><h1>404 Not Found</h1></center>"
        "<hr><center>"
        + name + "</center></body></html>";
}

int32_t NotFoundServlet::handle(phase0::http::HttpRequest::ptr request,
                                phase0::http::HttpResponse::ptr response,
                                phase0::http::HttpSession::ptr session)
{
    response->setStatus(phase0::http::HttpStatus::NOT_FOUND);
    response->setHeader("Server", "phase0/1.0.0");
    response->setHeader("Content-Type", "text/html");
    response->setBody(m_content);
    return 0;
}

}  // namespace http
}  // namespace phase0