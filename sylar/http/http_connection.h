#ifndef __SYLAR_HTTP_CONNECTION_H__
#define __SYLAR_HTTP_CONNECTION_H__

#include "sylar/socket_stream.h"
#include "http.h"
#include "sylar/uri.h"
#include "sylar/thread.h"
#include <string>
#include <list>
#include <stdint.h>
#include <atomic>

namespace sylar {
namespace http {

struct HttpResult {
    typedef std::shared_ptr<HttpResult> ptr;
    enum class Status {
        OK = 0,                         // 正常
        INVALID_URL = 1,                // 非法 URL
        INVALID_HOST = 2,               // 无法解析 HOST
        CONNECT_FAILURE = 3,            // 连接失败
        SEND_CLOSE_BY_PEER = 4,         // 对端关闭
        SEND_SOCKET_ERROR = 5,          // 发送请求产生 Socket 错误
        TIMEOUT = 6,                    // 超时
        CREATE_SOCKET_ERROR = 7,        // 创建 Socket 失败
        POOL_GET_CONNECTION = 8,        // 从连接池中取连接失败
        POOL_INVALID_CONNECTION = 9,    // 无效的连接

    };

    HttpResult(int res, HttpResponse::ptr rsp, const std::string& err);

    std::string toString() const;

    int result;                     // 错误码
    HttpResponse::ptr response;     // HTTP 响应结果
    std::string error;              // 错误描述
};

class HttpConnectionPool;

class HttpConnection : public SocketStream {
friend class HttpConnectionPool;
public:
    typedef std::shared_ptr<HttpConnection> ptr;
    HttpConnection(Socket::ptr sock, bool owner = true);
    ~HttpConnection();
    HttpResponse::ptr recvResponse();  // 获取 HTTP Response 结构体
    int sendRequest(HttpRequest::ptr req);
    
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

    static HttpResult::ptr DoRequest(HttpMethod method, const std::string& url
                , int64_t timeout_ms, const std::map<std::string, std::string>& headers = {}
                , const std::string& body = "");

    static HttpResult::ptr DoRequest(HttpMethod method, Uri::ptr uri, int64_t timeout_ms
                , const std::map<std::string, std::string>& headers = {}
                , const std::string& body = "");
    
    static HttpResult::ptr DoRequest(HttpRequest::ptr req, Uri::ptr uri, int64_t timeout_ms);

private:
    uint64_t m_createdTime = 0;         // 创建时间, 供连接池用
    uint64_t m_request = 0;             // request 计数，供连接池用
};

// http connecion 连接池
class HttpConnectionPool {
public:
    typedef std::shared_ptr<HttpConnectionPool> ptr;
    typedef Mutex MutexType;

    HttpConnectionPool(const std::string& host, const std::string& vhost, uint32_t port,
            uint32_t max_size, uint32_t max_alive_time, uint32_t max_request);
    
    HttpConnection::ptr getConnection();


    HttpResult::ptr doRequest(HttpMethod method, const std::string& url
                , int64_t timeout_ms, const std::map<std::string, std::string>& headers = {}
                , const std::string& body = "");

    HttpResult::ptr doRequest(HttpMethod method, Uri::ptr uri, int64_t timeout_ms
                , const std::map<std::string, std::string>& headers = {}
                , const std::string& body = "");
    
    HttpResult::ptr doRequest(HttpRequest::ptr req, int64_t timeout_ms);
    
    // Get 请求
    HttpResult::ptr doGet(const std::string& url
                , int64_t timeout_ms, const std::map<std::string, std::string>& headers = {}
                , const std::string& body = "");
    
    // Get 请求
    HttpResult::ptr doGet(Uri::ptr uri
                , int64_t timeout_ms, const std::map<std::string, std::string>& headers = {}
                , const std::string& body = "");
    
    // Post 请求
    HttpResult::ptr doPost(const std::string& url
                , int64_t timeout_ms, const std::map<std::string, std::string>& headers = {}
                , const std::string& body = "");
    
    // Post 请求
    HttpResult::ptr doPost(Uri::ptr uri
                , int64_t timeout_ms, const std::map<std::string, std::string>& headers = {}
                , const std::string& body = "");
private:
    static void ReleasePtr(HttpConnection *ptr, HttpConnectionPool *pool);
private:
    std::string m_host;
    std::string m_vhost;
    uint32_t m_port;
    uint32_t m_maxSize;
    uint32_t m_maxAliveTime;
    uint32_t m_maxRequest;

    MutexType m_mutex;
    std::list<HttpConnection*> m_conns;
    std::atomic<int32_t> m_total = {0};
};

}
}

#endif  // __SYLAR_HTTP_CONNECTION_H__