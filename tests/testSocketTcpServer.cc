#include "LogMarco.h"
#include "iomanager.h"
#include "socket.h"

void test_tcp_server()
{
    int ret;

    auto addr = phase0::Address::LookupAnyIPAddress("0.0.0.0:12345");
    PHASE0_ASSERT(addr);

    auto socket = phase0::Socket::CreateTCPSocket();
    PHASE0_ASSERT(socket);

    ret = socket->bind(addr);
    PHASE0_ASSERT(ret);

    P0ROOT_LOG_INFO() << "bind success";

    ret = socket->listen();
    PHASE0_ASSERT(ret);

    P0ROOT_LOG_INFO() << socket->toString();
    P0ROOT_LOG_INFO() << "listening...";

    while (1)
    {
        auto client = socket->accept();
        PHASE0_ASSERT(client);
        P0ROOT_LOG_INFO() << "new client: " << client->toString();
        client->send("hello world", strlen("hello world"));
        client->close();
    }
}

int main(int argc, char* argv[])
{
    phase0::IOManager iom(2);
    iom.schedule(&test_tcp_server);

    return 0;
}