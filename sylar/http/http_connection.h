#ifndef __SYLAR_HTTP_CONNECTION_H__
#define __SYLAR_HTTP_CONNECTION_H__

#include "sylar/socket_stream.h"
#include "http.h"
#include "sylar/uri.h"

namespace sylar {
namespace http {

struct HttpResult {
    typedef std::shared_ptr<HttpResult> ptr;
    enum class Status {
        OK = 0,
        INVALID_URL = 1,
        INVALID_HOST = 2,
        CREATE_SOCKET_ERROR = 3,
        CONNECT_FAILURE = 4,
        SEND_CLOSE_BY_PEER = 5,
        SEND_SOCKET_ERROR = 6,
        TIMEOUT = 7,

    };

    HttpResult(int res, HttpResponse::ptr rsp, const std::string& err)
        :result(res)
        ,response(rsp)
        ,error(err) {
    }

    int result;
    HttpResponse::ptr response;
    std::string error;
};

class HttpConnection : public SocketStream {
public:
    typedef std::shared_ptr<HttpConnection> ptr;
    HttpConnection(Socket::ptr sock, bool owner = true);

    HttpResponse::ptr recvResponse();  // 获取 HTTP Response 结构体
    int sendRequest(HttpRequest::ptr req);
    
    static HttpResult::ptr DoRequest(HttpMethod method, const std::string& url
                , int64_t timeout_ms, const std::map<std::string, std::string>& headers = {}
                , const std::string& body = "");

    static HttpResult::ptr DoRequest(HttpMethod method, Uri::ptr uri, int64_t timeout_ms
                , const std::map<std::string, std::string>& headers = {}
                , const std::string& body = "");
    
    static HttpResult::ptr DoRequest(HttpRequest::ptr req, Uri::ptr uri, int64_t timeout_ms);
    
    // Get 请求
    static HttpResult::ptr DoGet(const std::string& url
                , int64_t timeout_ms, const std::map<std::string, std::string>& headers = {}
                , const std::string& body = "");
    
    // Get 请求
    static HttpResult::ptr DoGet(Uri::ptr uri
                , int64_t timeout_ms, const std::map<std::string, std::string>& headers = {}
                , const std::string& body = "");
    
    // Post 请求
    static HttpResult::ptr DoPost(const std::string& url
                , int64_t timeout_ms, const std::map<std::string, std::string>& headers = {}
                , const std::string& body = "");
    
    // Post 请求
    static HttpResult::ptr DoPost(Uri::ptr uri
                , int64_t timeout_ms, const std::map<std::string, std::string>& headers = {}
                , const std::string& body = "");

};

}
}

#endif  // __SYLAR_HTTP_CONNECTION_H__