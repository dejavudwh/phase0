/**
 * @file test_tcp_server.cc
 * @brief TcpServer类测试
 * @version 0.1
 * @date 2021-09-18
 */
#include "tcpserver.h"

/**
 * @brief 自定义TcpServer类，重载handleClient方法
 */
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
    client->recv(&buf[0], buf.length());  // 这里有读超时，由tcp_server.read_timeout配置项进行配置，默认120秒
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