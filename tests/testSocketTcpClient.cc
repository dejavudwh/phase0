/**
 * @file test_socket_tcp_client.cc
 * @brief 测试Socket类，tcp客户端
 * @version 0.1
 * @date 2021-09-18
 */
#include "LogMarco.h"
#include "iomanager.h"
#include "socket.h"

void test_tcp_client()
{
    int ret;

    auto socket = phase0::Socket::CreateTCPSocket();
    PHASE0_ASSERT(socket);

    auto addr = phase0::Address::LookupAnyIPAddress("0.0.0.0:12345");
    PHASE0_ASSERT(addr);

    ret = socket->connect(addr);
    PHASE0_ASSERT(ret);

    P0ROOT_LOG_INFO() << "connect success, peer address: " << socket->getRemoteAddress()->toString();

    std::string buffer;
    buffer.resize(1024);
    socket->recv(&buffer[0], buffer.size());
    P0ROOT_LOG_INFO() << "recv: " << buffer;
    socket->close();

    return;
}

int main(int argc, char* argv[])
{
    phase0::IOManager iom;
    iom.schedule(&test_tcp_client);

    return 0;
}