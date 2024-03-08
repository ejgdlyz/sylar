#include "sylar/http/http_server.h"
#include "sylar/log.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();
sylar::IOManager::ptr worker;

void run() {
    g_logger->setLevel(sylar::LogLevel::INFO);
    sylar::Address::ptr addr = sylar::Address::LookupAnyIPAddress("0.0.0.0:8020");
    if (!addr) {
        SYLAR_LOG_ERROR(g_logger) << "get address error";
        return; 
    }
    // sylar::http::HttpServer::ptr http_server(new sylar::http::HttpServer(false));  // false: 短链接
    sylar::http::HttpServer::ptr http_server(new sylar::http::HttpServer(true, worker.get()));  // true:  长连接

    bool ssl = false;
    while(!http_server->bind(addr, ssl)) {
        SYLAR_LOG_ERROR(g_logger) << "bind" << *addr << " failture";
        sleep(1);
    }

    if (ssl) {
        http_server->loadCertificates("/home/app/sylar/keys/server.crt", "/home/apps/soft/sylar/keys/server.key");
    }
    
    http_server->start();
}

int main(int argc, char const *argv[]) {
    sylar::IOManager iom(1);
    worker.reset(new sylar::IOManager(2, false));
    iom.schedule(run);
    return 0;
}

