#include "sylar/http/http_connection.h"
#include "sylar/log.h"
#include "sylar/iomanager.h"
#include <fstream>

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

// 连接池测试
void test_connection_pool() {
    sylar::http::HttpConnectionPool::ptr pool(new sylar::http::HttpConnectionPool("www.sylar.top", "", false, 80, 10, 1000 * 30, 5));

    sylar::IOManager::GetThis()->addTimer(1000, [pool](){  // 每秒发一次
        auto r = pool->doGet("/", 300);
        SYLAR_LOG_INFO(g_logger) << std::endl 
        << r->toString(); 
    }, true);
}

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

// HttpConnection 接口简化测试
void run_test_http_connection_simple() {
    SYLAR_LOG_INFO(g_logger) << "===========================";

    sylar::http::HttpResult::ptr res = sylar::http::HttpConnection::DoGet("http://www.sylar.top/blog/", 3000);
    SYLAR_LOG_INFO(g_logger) << "result = " << res->result 
            << " status = " << res->error
            << " rsp = " << (res->response ? res->response->toString() : ""); 


    // 测试连接池
    test_connection_pool();
}

// https 测试
void test_https() {
    auto res = sylar::http::HttpConnection::DoGet("https://www.baidu.com", 300);
    SYLAR_LOG_INFO(g_logger) << "result = " << res->result
            << " error=" << res->error
            << " rsp=" << (res->response ? res->response->toString() : "");

    auto pool = sylar::http::HttpConnectionPool::Create("https://www.baidu.com", "", 10, 1000 * 30, 5);
    sylar::IOManager::GetThis()->addTimer(1000, [pool](){
        auto res = pool->doGet("/", 300);
        SYLAR_LOG_INFO(g_logger) << res->toString();
    }, true);
}

int main(int argc, char const *argv[]) {
    // sylar::IOManager iom(2);
    sylar::IOManager iom;
    
    // iom.schedule(run_test_http_connection);
    
    // iom.schedule(run_test_http_connection_simple);
    
    iom.schedule(test_https);
    return 0;
}
