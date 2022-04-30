#include "tcpserver.h"

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