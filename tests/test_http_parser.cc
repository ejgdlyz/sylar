#include "sylar/http/http_parser.h"
#include "sylar/log.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

const char test_requeset_data[] = "POST / HTTP/1.1\r\n"
                                "Host: www.sylar.top\r\n"
                                "Content-Length: 10\r\n\r\n"
                                "1234567890";

void test_http_request() {
    sylar::http::HttpRequestParser parser;
    std::string tmp = test_requeset_data;
    size_t s = parser.execute(&tmp[0], tmp.size(), 0);
    SYLAR_LOG_INFO(g_logger) << "execute header_len=" << s << " has_error=" << parser.hasError()
            << " is_finished=" << parser.isFinished() << " toatal=" << tmp.size()
            << "content-length=" << parser.getContentLength(); 
    tmp.resize(tmp.size() - s);
    SYLAR_LOG_INFO(g_logger) << parser.getData()->toString();
    SYLAR_LOG_INFO(g_logger) << tmp;

}

const char test_response_data[] = "HTTP/1.0 200 OK\r\n"
                                "Accept-Ranges: bytes\r\n"
                                "Cache-Control: no-cache\r\n"
                                "Content-Length: 10\r\n"
                                "Content-Type: text/html\r\n"
                                "Date: Sat, 30 Dec 2023 13:57:53 GMT\r\n"
                                "Pragma: no-cache\r\n"
                                "Server: BWS/1.1\r\n\r\n"
                                "1234567890";
void test_http_response() {
    sylar::http::HttpResponseParser parser;
    std::string tmp = test_response_data;
    size_t s = parser.execute(&tmp[0], tmp.size());
    SYLAR_LOG_INFO(g_logger) << "execute header_len=" << s << " has_error=" << parser.hasError()
            << " is_finished=" << parser.isFinished() << " toatal=" << tmp.size()
            << " content-length=" << parser.getContentLength(); 
    tmp.resize(tmp.size() - s);
    SYLAR_LOG_INFO(g_logger) << parser.getData()->toString();
    SYLAR_LOG_INFO(g_logger) << tmp;
}

int main(int argc, char const *argv[]) {
    test_http_request();
    SYLAR_LOG_INFO(g_logger) << "-------------------------------";
    test_http_response();
    return 0;
}
