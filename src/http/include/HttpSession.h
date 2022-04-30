#pragma once

#include "SocketStream.h"
#include "http.h"

namespace phase0
{
namespace http
{
class HttpSession : public SocketStream
{
public:
    using ptr = std::shared_ptr<HttpSession>;

    HttpSession(Socket::ptr sock, bool owner = true);

    HttpRequest::ptr recvRequest();
    int sendResponse(HttpResponse::ptr rsp);
};

}  // namespace http
}  // namespace phase0