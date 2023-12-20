#include "sylar/socket.h"
#include "sylar/sylar.h"
#include "sylar/iomanager.h"
#include "sylar/macro.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_socket() {
    sylar::IPAddress::ptr addr = sylar::Address::LookupAnyIPAddress("www.baidu.com");
    if (!addr) {
        SYLAR_LOG_ERROR(g_logger) << "fail to get address";
        return;
    }
    SYLAR_LOG_INFO(g_logger) << "address: " << addr->toString();

    sylar::Socket::ptr sock = sylar::Socket::CreateTCP(addr);
    addr->setPort(80);
    if (!sock->connect(addr)) {
        SYLAR_LOG_ERROR(g_logger) << "fail to connect";
        return;
    }
    SYLAR_LOG_INFO(g_logger) << "connected: " << addr->toString();

    const char buf[] = "GET / HTTP/1.1\r\n\r\n";
    int rt = sock->send(buf, sizeof(buf));
    SYLAR_ASSERT2(rt > 0, "fail to send");

    std::string buffs;
    buffs.resize(4096);
    rt = sock->recv(&buffs[0], buffs.size());
    SYLAR_ASSERT2(rt > 0, "fail to recv");

    buffs.resize(rt);
    SYLAR_LOG_INFO(g_logger) << buffs;

}

int main(int argc, char const *argv[])
{
    sylar::IOManager iom;
    iom.schedule(&test_socket);
    return 0;
}
