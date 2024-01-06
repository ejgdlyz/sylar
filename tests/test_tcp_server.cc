#include "sylar/tcp_server.h"
#include "sylar/iomanager.h"
#include "sylar/log.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void run() {
    std::vector<sylar::Address::ptr> addrs;
    auto addr = sylar::Address::LookupAny("0.0.0.0:8033");
    addrs.push_back(addr);
    // auto unix_addr = sylar::UnixAddress::ptr(new sylar::UnixAddress("/tmp/unix_addr"));
    // addrs.push_back(unix_addr);

    sylar::TcpServer::ptr tcp_server(new sylar::TcpServer);
    std::vector<sylar::Address::ptr> failed_addrs;
    while(!tcp_server->bind(addrs, failed_addrs)) {
        sleep(2);
    }
    tcp_server->start();
}

int main(int argc, char const *argv[]) {
    sylar::IOManager iom(2);
    iom.schedule(run);
    return 0;
}
