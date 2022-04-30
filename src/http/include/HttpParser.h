#pragma once

#include "http.h"

namespace phase0
{
namespace http
{
class HttpRequestParser
{
public:
    using ptr = std::shared_ptr<HttpRequestParser>;

    HttpRequestParser();

    size_t execute(char* data, size_t len);

    int isFinished() const { return m_finished; }

    void setFinished(bool v) { m_finished = v; }

    int hasError() const { return !!m_error; }
    void setError(int v) { m_error = v; }

    HttpRequest::ptr getData() const { return m_data; }

    const http_parser& getParser() const { return m_parser; }

    const std::string& getField() const { return m_field; }
    void setField(const std::string& v) { m_field = v; }

public:
    static uint64_t GetHttpRequestBufferSize();
    static uint64_t GetHttpRequestMaxBodySize();

private:
    http_parser m_parser;
    HttpRequest::ptr m_data;
    int m_error;
    bool m_finished;
    std::string m_field;
};

class HttpResponseParser
{
public:
    using ptr = std::shared_ptr<HttpResponseParser>;

    HttpResponseParser();

    size_t execute(char* data, size_t len);

    int isFinished() const { return m_finished; }
    void setFinished(bool v) { m_finished = v; }

    int hasError() const { return !!m_error; }
    void setError(int v) { m_error = v; }

    HttpResponse::ptr getData() const { return m_data; }

    const http_parser& getParser() const { return m_parser; }

    const std::string& getField() const { return m_field; }
    void setField(const std::string& v) { m_field = v; }

public:
    static uint64_t GetHttpResponseBufferSize();
    static uint64_t GetHttpResponseMaxBodySize();

private:
    http_parser m_parser;
    HttpResponse::ptr m_data;
    int m_error;
    bool m_finished;
    std::string m_field;
};

}  // namespace http
}  // namespace phase0
