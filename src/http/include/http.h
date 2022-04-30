#pragma once

#include <boost/lexical_cast.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "http_parser.h"

namespace phase0
{
namespace http
{
enum class HttpMethod
{
#define XX(num, name, string) name = num,
    HTTP_METHOD_MAP(XX)
#undef XX
        INVALID_METHOD
};

enum class HttpStatus
{
#define XX(code, name, desc) name = code,
    HTTP_STATUS_MAP(XX)
#undef XX
};

HttpMethod StringToHttpMethod(const std::string& m);
HttpMethod CharsToHttpMethod(const char* m);

const char* HttpMethodToString(const HttpMethod& m);
const char* HttpStatusToString(const HttpStatus& s);

struct CaseInsensitiveLess
{
    bool operator()(const std::string& lhs, const std::string& rhs) const;
};

template <class MapType, class T>
bool checkGetAs(const MapType& m, const std::string& key, T& val, const T& def = T())
{
    auto it = m.find(key);
    if (it == m.end())
    {
        val = def;
        return false;
    }
    try
    {
        val = boost::lexical_cast<T>(it->second);
        return true;
    }
    catch (...)
    {
        val = def;
    }
    return false;
}

template <class MapType, class T>
T getAs(const MapType& m, const std::string& key, const T& def = T())
{
    auto it = m.find(key);
    if (it == m.end())
    {
        return def;
    }
    try
    {
        return boost::lexical_cast<T>(it->second);
    }
    catch (...)
    {
    }
    return def;
}

class HttpResponse;

class HttpRequest
{
public:
    using ptr = std::shared_ptr<HttpRequest>;
    using MapType = std::map<std::string, std::string, CaseInsensitiveLess>;

    HttpRequest(uint8_t version = 0x11, bool close = true);

    std::shared_ptr<HttpResponse> createResponse();

    HttpMethod getMethod() const { return m_method; }
    uint8_t getVersion() const { return m_version; }

    const std::string& getPath() const { return m_path; }
    const std::string& getQuery() const { return m_query; }
    const std::string& getBody() const { return m_body; }
    const MapType& getHeaders() const { return m_headers; }
    const MapType& getParams() const { return m_params; }
    const MapType& getCookies() const { return m_cookies; }

    void setMethod(HttpMethod v) { m_method = v; }
    void setVersion(uint8_t v) { m_version = v; }
    void setPath(const std::string& v) { m_path = v; }
    void setQuery(const std::string& v) { m_query = v; }
    void setFragment(const std::string& v) { m_fragment = v; }
    void setBody(const std::string& v) { m_body = v; }

    void appendBody(const std::string& v) { m_body.append(v); }

    bool isClose() const { return m_close; }
    void setClose(bool v) { m_close = v; }

    bool isWebsocket() const { return m_websocket; }
    void setWebsocket(bool v) { m_websocket = v; }

    void setHeaders(const MapType& v) { m_headers = v; }
    void setParams(const MapType& v) { m_params = v; }
    void setCookies(const MapType& v) { m_cookies = v; }

    std::string getHeader(const std::string& key, const std::string& def = "") const;
    std::string getParam(const std::string& key, const std::string& def = "");
    std::string getCookie(const std::string& key, const std::string& def = "");

    void setHeader(const std::string& key, const std::string& val);
    void setParam(const std::string& key, const std::string& val);
    void setCookie(const std::string& key, const std::string& val);

    void delHeader(const std::string& key);
    void delParam(const std::string& key);
    void delCookie(const std::string& key);

    bool hasHeader(const std::string& key, std::string* val = nullptr);
    bool hasParam(const std::string& key, std::string* val = nullptr);
    bool hasCookie(const std::string& key, std::string* val = nullptr);

    template <class T>
    bool checkGetHeaderAs(const std::string& key, T& val, const T& def = T())
    {
        return checkGetAs(m_headers, key, val, def);
    }

    template <class T>
    T getHeaderAs(const std::string& key, const T& def = T())
    {
        return getAs(m_headers, key, def);
    }

    template <class T>
    bool checkGetParamAs(const std::string& key, T& val, const T& def = T())
    {
        initQueryParam();
        initBodyParam();
        return checkGetAs(m_params, key, val, def);
    }

    template <class T>
    T getParamAs(const std::string& key, const T& def = T())
    {
        initQueryParam();
        initBodyParam();
        return getAs(m_params, key, def);
    }

    template <class T>
    bool checkGetCookieAs(const std::string& key, T& val, const T& def = T())
    {
        initCookies();
        return checkGetAs(m_cookies, key, val, def);
    }

    template <class T>
    T getCookieAs(const std::string& key, const T& def = T())
    {
        initCookies();
        return getAs(m_cookies, key, def);
    }

    std::ostream& dump(std::ostream& os) const;

    std::string toString() const;

    void initQueryParam();
    void initBodyParam();
    void initCookies();
    void init();

private:
    HttpMethod m_method;
    uint8_t m_version;
    bool m_close;
    bool m_websocket;
    uint8_t m_parserParamFlag;
    std::string m_url;
    std::string m_path;
    std::string m_query;
    std::string m_fragment;
    std::string m_body;
    MapType m_headers;
    MapType m_params;
    MapType m_cookies;
};

class HttpResponse
{
public:
    using ptr = std::shared_ptr<HttpResponse>;
    using MapType = std::map<std::string, std::string, CaseInsensitiveLess>;

    HttpResponse(uint8_t version = 0x11, bool close = true);

    HttpStatus getStatus() const { return m_status; }
    uint8_t getVersion() const { return m_version; }

    const std::string& getBody() const { return m_body; }
    const std::string& getReason() const { return m_reason; }
    const MapType& getHeaders() const { return m_headers; }
    void setStatus(HttpStatus v) { m_status = v; }
    void setVersion(uint8_t v) { m_version = v; }
    void setBody(const std::string& v) { m_body = v; }

    void appendBody(const std::string& v) { m_body.append(v); }

    void setReason(const std::string& v) { m_reason = v; }
    void setHeaders(const MapType& v) { m_headers = v; }

    bool isClose() const { return m_close; }
    void setClose(bool v) { m_close = v; }
    bool isWebsocket() const { return m_websocket; }
    void setWebsocket(bool v) { m_websocket = v; }

    std::string getHeader(const std::string& key, const std::string& def = "") const;
    void setHeader(const std::string& key, const std::string& val);
    void delHeader(const std::string& key);

    template <class T>
    bool checkGetHeaderAs(const std::string& key, T& val, const T& def = T())
    {
        return checkGetAs(m_headers, key, val, def);
    }

    template <class T>
    T getHeaderAs(const std::string& key, const T& def = T())
    {
        return getAs(m_headers, key, def);
    }

    std::ostream& dump(std::ostream& os) const;

    std::string toString() const;

    void setRedirect(const std::string& uri);

    void setCookie(const std::string& key,
                   const std::string& val,
                   time_t expired = 0,
                   const std::string& path = "",
                   const std::string& domain = "",
                   bool secure = false);

private:
    HttpStatus m_status;
    uint8_t m_version;
    bool m_close;
    bool m_websocket;
    std::string m_body;
    std::string m_reason;
    MapType m_headers;
    std::vector<std::string> m_cookies;
};

/**
 * @brief 流式输出HttpRequest
 * @param[in, out] os 输出流
 * @param[in] req HTTP请求
 * @return 输出流
 */
std::ostream& operator<<(std::ostream& os, const HttpRequest& req);

/**
 * @brief 流式输出HttpResponse
 * @param[in, out] os 输出流
 * @param[in] rsp HTTP响应
 * @return 输出流
 */
std::ostream& operator<<(std::ostream& os, const HttpResponse& rsp);

}  // namespace http
}  // namespace phase0
