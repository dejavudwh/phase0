#include "address.h"
#include "LogMarco.h"

const char *family2str(int family) {
    switch (family) {
#define XX(name) case (name): return #name;
        XX(AF_INET)
        XX(AF_INET6)
        XX(AF_UNIX)
        XX(AF_UNSPEC)
#undef XX
    default:
        return "UNKNOWN";
    }
}

/**
 * @brief 查询所有网卡
 * @param[in] family 地址类型
 */
void test_ifaces(int family) {
    P0ROOT_LOG_INFO() << "test_ifaces: " << family2str(family);

    std::multimap<std::string, std::pair<phase0::Address::ptr, uint32_t>> results;
    bool v = phase0::Address::GetInterfaceAddresses(results, family);
    if (!v) {
        P0ROOT_LOG_ERROR() << "GetInterfaceAddresses fail";
        return;
    }
    for (auto &i : results) {
        P0ROOT_LOG_INFO() << i.first << " - " << i.second.first->toString() << " - "
                                 << i.second.second;
    }
    
    P0ROOT_LOG_INFO() << "\n";
}

/**
 * @brief 查询指定网卡
 * @param[in] iface 网卡名称
 * @param[in] family 地址类型
 */
void test_iface(const char *iface, int family) {
    P0ROOT_LOG_INFO() << "test_iface: " << iface << ", " << family2str(family);

    std::vector<std::pair<phase0::Address::ptr, uint32_t>> result;
    bool v = phase0::Address::GetInterfaceAddresses(result, iface, family);
    if(!v) {
        P0ROOT_LOG_ERROR() << "GetInterfaceAddresses fail";
        return;
    }
    for(auto &i : result) {
        P0ROOT_LOG_INFO() << i.first->toString() << " - " << i.second;
    }

    P0ROOT_LOG_INFO() << "\n";
}

/**
 * @brief 测试网络地址解析
 * @param[] host 网络地址描述，包括字符串形式的域名/主机名或是数字格式的IP地址，支持端口和服务名解析
 * @note 这里没有区分不同的套接字类型，所以会有重复值
 */
void test_lookup(const char *host) {
    P0ROOT_LOG_INFO() << "test_lookup: " << host;

    P0ROOT_LOG_INFO() <<"Lookup:";
    std::vector<phase0::Address::ptr> results;
    bool v = phase0::Address::Lookup(results, host, AF_INET);
    if(!v) {
        P0ROOT_LOG_ERROR() << "Lookup fail";
        return;
    }
    for(auto &i : results) {
        P0ROOT_LOG_INFO() << i->toString();
    }
    
    P0ROOT_LOG_INFO() <<"LookupAny:";
    auto addr2 = phase0::Address::LookupAny(host);
    P0ROOT_LOG_INFO() << addr2->toString();

    P0ROOT_LOG_INFO() <<"LookupAnyIPAddress:";
    auto addr1 = phase0::Address::LookupAnyIPAddress(host);
    P0ROOT_LOG_INFO() << addr1->toString();

    P0ROOT_LOG_INFO() << "\n";
}

/**
 * @brief IPv4地址类测试
 */
void test_ipv4() {
    P0ROOT_LOG_INFO() << "test_ipv4";

    auto addr = phase0::IPAddress::Create("192.168.1.120");
    if (!addr) {
        P0ROOT_LOG_ERROR() << "IPAddress::Create error";
        return;
    }
    P0ROOT_LOG_INFO() << "addr: " << addr->toString();
    P0ROOT_LOG_INFO() << "family: " << family2str(addr->getFamily());  
    P0ROOT_LOG_INFO() << "port: " << addr->getPort();  
    P0ROOT_LOG_INFO() << "addr length: " << addr->getAddrLen(); 

    P0ROOT_LOG_INFO() << "broadcast addr: " << addr->broadcastAddress(24)->toString();
    P0ROOT_LOG_INFO() << "network addr: " << addr->networkAddress(24)->toString();
    P0ROOT_LOG_INFO() << "subnet mask addr: " << addr->subnetMask(24)->toString();

    P0ROOT_LOG_INFO() << "\n";
}

/**
 * @brief IPv6地址类测试
 */
void test_ipv6() {
    P0ROOT_LOG_INFO() << "test_ipv6";

    auto addr = phase0::IPAddress::Create("fe80::215:5dff:fe88:d8a");
    if (!addr) {
        P0ROOT_LOG_ERROR() << "IPAddress::Create error";
        return;
    }
    P0ROOT_LOG_INFO() << "addr: " << addr->toString();
    P0ROOT_LOG_INFO() << "family: " << family2str(addr->getFamily());  
    P0ROOT_LOG_INFO() << "port: " << addr->getPort();  
    P0ROOT_LOG_INFO() << "addr length: " << addr->getAddrLen(); 

    P0ROOT_LOG_INFO() << "broadcast addr: " << addr->broadcastAddress(64)->toString();
    P0ROOT_LOG_INFO() << "network addr: " << addr->networkAddress(64)->toString();
    P0ROOT_LOG_INFO() << "subnet mask addr: " << addr->subnetMask(64)->toString();
    P0ROOT_LOG_INFO() << "\n";
}

/**
 * @brief Unix套接字解析
 */
void test_unix() {
    P0ROOT_LOG_INFO() << "test_unix";
    
    auto addr = phase0::UnixAddress("/tmp/test_unix.sock");
    P0ROOT_LOG_INFO() << "addr: " << addr.toString();
    P0ROOT_LOG_INFO() << "family: " << family2str(addr.getFamily());  
    P0ROOT_LOG_INFO() << "path: " << addr.getPath(); 
    P0ROOT_LOG_INFO() << "addr length: " << addr.getAddrLen(); 

    P0ROOT_LOG_INFO() << "\n";
}

int main(int argc, char *argv[]) {
    // 获取本机所有网卡的IPv4地址和IPv6地址以及掩码长度
    test_ifaces(AF_INET);
    test_ifaces(AF_INET6);

    // 获取本机eth0网卡的IPv4地址和IPv6地址以及掩码长度
    test_iface("eth0", AF_INET);
    test_iface("eth0", AF_INET6);

    // ip域名服务解析
    test_lookup("127.0.0.1");
    test_lookup("127.0.0.1:80");
    test_lookup("127.0.0.1:http");
    test_lookup("127.0.0.1:ftp");
    test_lookup("localhost");
    test_lookup("localhost:80");
    test_lookup("www.baidu.com");
    test_lookup("www.baidu.com:80");
    test_lookup("www.baidu.com:http");

    // IPv4地址类测试
    test_ipv4();

    // IPv6地址类测试
    test_ipv6();

    // Unix套接字地址类测试
    test_unix();

    return 0;
}