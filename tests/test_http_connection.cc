#include "sylar/http/http_connection.h"
#include "sylar/log.h"
#include "sylar/iomanager.h"
#include <fstream>

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void run_test_http_connection() {
    sylar::Address::ptr addr = sylar::Address::LookupAnyIPAddress("www.sylar.top:80");
    // sylar::Address::ptr addr = sylar::Address::LookupAnyIPAddress("127.0.0.1:9800");

    if (!addr) {
        SYLAR_LOG_INFO(g_logger) << "get addr error";
        return;
    }

    sylar::Socket::ptr sock = sylar::Socket::CreateTCP(addr);
    bool rt = sock->connect(addr);
    if (!rt) {
        SYLAR_LOG_INFO(g_logger) << "connect " << *addr << " failed";
        return;
    }

    sylar::http::HttpConnection::ptr conn(new sylar::http::HttpConnection(sock));
    sylar::http::HttpRequest::ptr req(new sylar::http::HttpRequest);  // 创建一个默认的 request
    req->setPath("/blog/");
    req->setHeader("host", "www.sylar.top");  // 默认没有 host, 手动添加
    SYLAR_LOG_INFO(g_logger) << "req: " << std::endl << *req;

    conn->sendRequest(req);
    sylar::http::HttpResponse::ptr rsp = conn->recvResponse();
    if (!rsp) {
        SYLAR_LOG_INFO(g_logger) << "recv response error";
        return;
    }
    SYLAR_LOG_INFO(g_logger) << "rsp: " << std::endl << *rsp;
    // std::ofstream ofs("http.txt");
    // ofs << *rsp;
}

int main(int argc, char const *argv[]) {
    // sylar::IOManager iom(2);
    sylar::IOManager iom;
    iom.schedule(run_test_http_connection);
    return 0;
}
