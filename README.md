# phase0

> C++ server framework for study C++ and Socket Programming

## Usage

### Servlet and HTTP Server

```cpp
void run()
{
    phase0::http::HttpServer::ptr server(new phase0::http::HttpServer(true));
    phase0::Address::ptr addr = phase0::Address::LookupAnyIPAddress("0.0.0.0:8080");
    while (!server->bind(addr))
    {
        sleep(2);
    }
    auto sd = server->getServletDispatch();
    sd->addGlobServlet("/phase0/*",
                       [](phase0::http::HttpRequest::ptr req,
                          phase0::http::HttpResponse::ptr rsp,
                          phase0::http::HttpSession::ptr session) {
                           rsp->setBody("Glob:\r\n" + req->toString());
                           return 0;
                       });
    server->start();
}

int main(int argc, char** argv)
{
    phase0::IOManager iom(1, true, "main");
    phase0::IOManager::ptr worker;
    worker.reset(new phase0::IOManager(3, false, "worker"));
    iom.schedule(run);
    return 0;
}
```


### TCP Server

```cpp
class MyTcpServer : public phase0::TcpServer
{
protected:
    virtual void handleClient(phase0::Socket::ptr client) override;
};

void MyTcpServer::handleClient(phase0::Socket::ptr client)
{
    P0ROOT_LOG_INFO() << "new client: " << client->toString();
    static std::string buf;
    buf.resize(4096);
    client->recv(&buf[0], buf.length());  
    P0ROOT_LOG_INFO() << "recv: " << buf;
    client->close();
}

void run()
{
    phase0::TcpServer::ptr server(new MyTcpServer);
    auto addr = phase0::Address::LookupAny("0.0.0.0:12345");
    PHASE0_ASSERT(addr);
    std::vector<phase0::Address::ptr> addrs;
    addrs.push_back(addr);

    std::vector<phase0::Address::ptr> fails;
    while (!server->bind(addrs, fails))
    {
        sleep(2);
    }

    P0ROOT_LOG_INFO() << "bind success, " << server->toString();

    server->start();
}

int main(int argc, char* argv[])
{
    phase0::IOManager iom(3);
    iom.schedule(&run);

    return 0;
}
```

## Module

### Logger Module

> Originally asynchronous streaming logs, now synchronous Support for colours, custom formats, setting log levels, multiple log separations (e.g. Root log and System log) and custom output locations (default output to console).

```cpp
P0ROOT_LOG_INFO() << "THIS INFO LOG";
P0ROOT_LOG_DEBUG() << "THIS DEBUG LOG";
P0ROOT_LOG_WARN() << "THIS WARN LOG";
P0ROOT_LOG_ERROR() << "THIS ERROR LOG";
P0ROOT_LOG_FATAL() << "THIS FATAL LOG";

P0SYS_LOG_INFO() << "THIS INFO LOG";
P0SYS_LOG_DEBUG() << "THIS DEBUG LOG";
P0SYS_LOG_WARN() << "THIS WARN LOG";
P0SYS_LOG_ERROR() << "THIS ERROR LOG";
P0SYS_LOG_FATAL() << "THIS FATAL LOG";
```

### Config Module

> Support for YAML configuration files, which require an API to be set up in code via an interface first, and then the configuration files can be serialised and deserialised(multiple types and custom types are supported).

```yaml
server:
    test_vec: [1, 2, 3]
```

```cpp
phase0::Config::lookup<vector<int>>("server.test_vec", {0, 0, 0}, "");

auto testInt = phase0::Config::lookup<vector<int>>("server.test_vec", {0, 0, 0}, "");
P0ROOT_LOG_DEBUG() <<testInt->toString();
```

### Fiber and Fiber scheduling modules

> Based on the ucontext implementation of Fiber, the Fiber scheduler is used to manage the scheduling of concurrent threads. The internal implementation is a thread pool, which supports switching between concurrent threads in multiple threads, and can also specify concurrent threads to execute in a fixed thread, an N-M model of concurrency scheduling.

> IO Fiber Scheduler, inherits from Fiber Scheduler, encapsulates epoll (Linux) and supports timer functions, supports adding, removing and cancelling socket read and write events.

### Hook

> The hook system-related API, combined with the Fiber module, allows synchronous calls to be executed asynchronously.

```cpp
sleep
usleep
nanosleep
socket
connect
accept
read
readv
recv
recvfrom
recvmsg
write
writev
send
sendto
sendmsg
close
fcntl
ioctl
getsockopt
setsockopt
```

### Address/Socket

> Encapsulates socket-related functions and provides all socket API functionality.

### TCPServer/HTTPServer/Servlet

> Refer to usage