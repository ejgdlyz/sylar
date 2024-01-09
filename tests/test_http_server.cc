#include "sylar/http/http_server.h"
#include "sylar/log.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void run() {
    sylar::http::HttpServer::ptr server(new sylar::http::HttpServer);
    sylar::Address::ptr addr = sylar::Address::LookupAnyIPAddress("0.0.0.0:8020");
    while (!server->bind(addr)) {
        sleep(2);
    }

    server->start();
}

int main(int argc, char const *argv[]) {
    // sylar::IOManager iom(2);
    sylar::IOManager iom;
    iom.schedule(run);
    return 0;
}
