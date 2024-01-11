#ifndef __SYLAR_HTTP_CONNECTION_H__
#define __SYLAR_HTTP_CONNECTION_H__

#include "sylar/socket_stream.h"
#include "http.h"
 
namespace sylar {
namespace http {

class HttpConnection : public SocketStream {
public:
    typedef std::shared_ptr<HttpConnection> ptr;
    HttpConnection(Socket::ptr sock, bool owner = true);

    HttpResponse::ptr recvResponse();  // 获取 HTTP Response 结构体
    int sendRequest(HttpRequest::ptr req);
    
private:
};

}
}

#endif  // __SYLAR_HTTP_CONNECTION_H__