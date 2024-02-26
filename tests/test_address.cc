#include "sylar/address.h"
#include "sylar/log.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_address() {
    std::vector<sylar::Address::ptr> addrs;
    // bool v = sylar::Address::Lookup(addrs, "www.baidu.com");
    // bool v = sylar::Address::Lookup(addrs, "www.baidu.com:80");
    bool v = sylar::Address::Lookup(addrs, "localhost:3080");
    // bool v = sylar::Address::Lookup(addrs, "www.baidu.com:http");  // http -> 80
    // bool v = sylar::Address::Lookup(addrs, "www.baidu.com:ftp");  // ftp -> 21


    if (!v) {
        SYLAR_LOG_ERROR(g_logger) << "lookup failure";
        return;
    }

    for (size_t i = 0; i < addrs.size(); ++i) {
        SYLAR_LOG_INFO(g_logger) << i << " - " << addrs[i]->toString();
    }

    auto addr = sylar::Address::LookupAny("localhost:4080");
    if (addr) {
        SYLAR_LOG_INFO(g_logger) << *addr;
    } else {
        SYLAR_LOG_ERROR(g_logger) << "error";
    }
}

void test_iface() {
    std::multimap<std::string, std::pair<sylar::Address::ptr, uint32_t>> results;
    bool v = sylar::Address::GetInterfaceAddresses(results);
    if (!v) {
        SYLAR_LOG_ERROR(g_logger) << "GetInterfaceAddresses fail";
        return;
    }

    for (auto& m: results) {
        SYLAR_LOG_INFO(g_logger) << m.first << " - " << m.second.first->toString() << " - " << m.second.second;
    }
}

void test_ipv4() {
    // auto addr = sylar::IPAddress::Create("www.baidu.com");
    auto addr = sylar::IPAddress::Create("127.0.0.8");

    if (addr) {
         SYLAR_LOG_INFO(g_logger) << addr->toString();
    }
}

void test_networkaddress() {
    const char* ip = "192.168.233.128";
    const int port = 80;
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_port = htons(port);
    int result = inet_pton(AF_INET, ip, &addr.sin_addr);
    if (result <= 0) {
        SYLAR_LOG_INFO(g_logger) << "inet_pton error";
    }
    sylar::IPv4Address::ptr ipv4(new sylar::IPv4Address(addr));
    int prefix_len = 24;
    SYLAR_LOG_INFO(g_logger) << "ipv4 address: " << ipv4->toString();
    SYLAR_LOG_INFO(g_logger) << "broadcast address: " << ipv4->broadcastAddress(prefix_len)->toString();
    SYLAR_LOG_INFO(g_logger) << "network address: " << ipv4->networkAddress(prefix_len)->toString();
    SYLAR_LOG_INFO(g_logger) << "subnet mask: " << ipv4->subnetMask(prefix_len)->toString();

}

int main() {
    // test();
    // test_iface();
    // test_ipv4();
    // test_networkaddress();

    test_address();
    return 0;
}