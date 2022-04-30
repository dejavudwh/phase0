#include "HttpServer.h"

#define XX(...) #__VA_ARGS__

phase0::IOManager::ptr worker;

void run()
{
    phase0::http::HttpServer::ptr server(new phase0::http::HttpServer(true));
    phase0::Address::ptr addr = phase0::Address::LookupAnyIPAddress("0.0.0.0:8080");
    while (!server->bind(addr))
    {
        sleep(2);
    }
    auto sd = server->getServletDispatch();
    sd->addServlet("/phase0/xx",
                   [](phase0::http::HttpRequest::ptr req,
                      phase0::http::HttpResponse::ptr rsp,
                      phase0::http::HttpSession::ptr session) {
                       rsp->setBody(req->toString());
                       return 0;
                   });

    sd->addGlobServlet("/phase0/*",
                       [](phase0::http::HttpRequest::ptr req,
                          phase0::http::HttpResponse::ptr rsp,
                          phase0::http::HttpSession::ptr session) {
                           rsp->setBody("Glob:\r\n" + req->toString());
                           return 0;
                       });

    sd->addGlobServlet("/phase0/*",
                       [](phase0::http::HttpRequest::ptr req,
                          phase0::http::HttpResponse::ptr rsp,
                          phase0::http::HttpSession::ptr session) {
                           rsp->setBody(XX(<html><head><title> 404 Not Found</ title></ head><body><center>
                                                   <h1> 404 Not Found</ h1></ center><hr><center> nginx
                                                   / 1.16.0
                                               < / center > </ body></ html> < !--a padding to disable MSIE
                                           and Chrome friendly error page-- > < !--a padding to disable MSIE
                                           and Chrome friendly error page-- > < !--a padding to disable MSIE
                                           and Chrome friendly error page-- > < !--a padding to disable MSIE
                                           and Chrome friendly error page-- > < !--a padding to disable MSIE
                                           and Chrome friendly error page-- > < !--a padding to disable MSIE
                                           and Chrome friendly error page-- >));
                           return 0;
                       });

    server->start();
}

int main(int argc, char** argv)
{
    phase0::IOManager iom(1, true, "main");
    worker.reset(new phase0::IOManager(3, false, "worker"));
    iom.schedule(run);
    return 0;
}