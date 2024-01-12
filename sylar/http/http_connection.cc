#include "http_connection.h"
#include "http_parser.h"
#include "sylar/log.h"

namespace sylar {
namespace http {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

HttpConnection::HttpConnection(Socket::ptr sock, bool owner) 
    : SocketStream(sock, owner) {
}

// 获取 HTTP Response 结构体
HttpResponse::ptr HttpConnection::recvResponse() {
    HttpResponseParser::ptr parser(new HttpResponseParser);
    
    // 申请一块缓冲区
    uint64_t buff_size = HttpResponseParser::GetHttpResponseBufferSize();
    // uint64_t buff_size = 10;
    std::shared_ptr<char> buffer(new char[buff_size + 1], 
        [](char* ptr){ delete[] ptr;});
    
    char* data = buffer.get();
    int offset = 0;  // execute 解析后 data 的剩余长度，
    do {
        int len = read(data + offset, buff_size - offset);  // 在 offset 后继续读取数据
        if (len <= 0) {
            close();
            return nullptr;
        }
        len += offset;      // 当前 data 长度 = 当前读取的 data 长度 + 上次剩余的数据长度
        data[len] = '\0';
        size_t nparse = parser->execute(data, len, false);  // nparse 为成功解析的字节数，且 data 会向前移动 nparse 个字节
        if (parser->hasError()) {
            close();
            return nullptr;
        }
        offset = len - nparse;  // 此时 data 剩余的数据为：当前 data 长度 (len) - execute 移除的数据长度 (nparse)
        if (offset == (int)buff_size) {  // 缓冲区满还未解析完
            close();
            return nullptr;
        }
        if (parser->isFinished()) {  // 解析结束
            break;
        }
    } while (true);

    auto& client_parser = parser->getParser();
    std::string body;
    // HTTP 响应是否被分块传输(chunked)
    if (client_parser.chunked) {
        int len = offset;  // Header 解析后的剩余数据长度
        do {
            // 得到到长度 length 如 "12b8\r\n" 中 12b8
            bool begin = true;
            do {
                if (!begin || len == 0) {
                    int rt = read(data + len, buff_size - len);
                    if (rt <= 0) {  // 出错
                        close();	
                        return nullptr;
                    }
                    len += rt;
                }
                data[len] = '\0';  // 要求末尾必须以 '\0' 结束
                size_t nparse = parser->execute(data, len, true);  // 解析 "12b8\r\n"，其大小为 3，并移动剩余数据
                if (parser->hasError()) {
                    close();	
                    return nullptr;
                }
                len -= nparse;
                if (len == (int)buff_size) {  // 解析了数据还是满
                    close();
                    return nullptr;
                }
                begin = false;
            } while (!parser->isFinished());
            
            // len -= 2;       // \r\n
            SYLAR_LOG_DEBUG(g_logger) << "content_len = " << client_parser.content_len;
            
            // client_parser.content_len 是已解析的数据大小，即去掉长度 12b8 和 \r\n 后的剩余大小，可能包含有重复数据
            if (client_parser.content_len + 2 <= len) {     
                body.append(data, client_parser.content_len);
                // 还有多余数据，移动到 data 首部开始
                memmove(data, data + client_parser.content_len + 2, len - client_parser.content_len - 2);
                len -= client_parser.content_len + 2;
            } else {
                body.append(data, len);     // len 是 data 真正剩余的大小，将 data 中剩余的 len 长度字节数据加入 body
                int left = client_parser.content_len - len + 2;
                while (left > 0) {
                    int rt = read(data, left > (int)buff_size ? (int)buff_size : left);
                    if (rt <= 0) {
                        close();
                        return nullptr;
                    }
                    body.append(data, rt);
                    left -= rt;
                }
                body.resize(body.size() - 2);
                len = 0;  // 此 chunck 读完，进入下一个 chunck 读
            }

        } while(!client_parser.chunks_done);

    } else {  // 未 chunked 响应体
        int64_t body_length = parser->getContentLength();  // 获得 header 中 "content-length" 对应的 body 长度
        if (body_length > 0) {
            body.resize(body_length);

            int len = 0;
            if (body_length >= offset) {     // body 长度 > 缓冲区剩余大小，将 data 中的数据全部复制到 body
                memcpy(&body[0], data, offset);
                len = offset;
            } else {                    // 否则，直接取 body_length
                memcpy(&body[0], data, body_length);
                len = body_length;
            }
            body_length -= offset;
            if (body_length > 0) {      // 缓冲区中的数据 < body_length，再从缓冲区读取数据直到满足 body_length
                if (readFixSize(&body[len], body_length) <= 0) {
                    close();
                    return nullptr;
                }
            }
        }
    }
    if (!body.empty()) {
        parser->getData()->setBody(body);   // 设置 body
    }
    return parser->getData();  // 返回解析后的 HttpResponse
}

int HttpConnection::sendRequest(HttpRequest::ptr req) {
    std::stringstream ss;
    ss << *req;
    std::string data = ss.str();
    return writeFixSize(data.c_str(), data.size());
}

HttpResult::ptr HttpConnection::DoRequest(HttpMethod method, const std::string& url
        , int64_t timeout_ms, const std::map<std::string, std::string>& headers
        , const std::string& body) {
    Uri::ptr uri = Uri::Create(url);
    if (!uri) {
        return std::make_shared<HttpResult>((int)HttpResult::Status::INVALID_URL, nullptr, "invalid_url: " + url);
    }

    return DoRequest(method, uri, timeout_ms, headers, body);
}

// 统一的封装，将 URL 中的参数结合 header，body等封装为一个 HttpRequest 对象
HttpResult::ptr HttpConnection::DoRequest(HttpMethod method, Uri::ptr uri, int64_t timeout_ms
        , const std::map<std::string, std::string>& headers
        , const std::string& body) {
    HttpRequest::ptr req = std::make_shared<HttpRequest>();
    req->setPath(uri->getPath());
    req->setMethod(method);

    bool has_host = false;
    for (auto& header: headers) {
        if (strcasecmp(header.first.c_str(), "connection") == 0) {
            if (strcasecmp(header.second.c_str(), "keep-alive") == 0) {
                req->setClose(false);  // 长连接
            }
            continue;
        }
        
        if (!has_host && strcasecmp(header.first.c_str(), "host") == 0) {
            has_host = !header.second.empty();
        }
        req->setHeader(header.first, header.second);
    }
    if (!has_host) {
        req->setHeader("host", uri->getHost());
    }

    req->setBody(body);
    return DoRequest(req, uri, timeout_ms);
}

// 最终所有方法的实现
HttpResult::ptr HttpConnection::DoRequest(HttpRequest::ptr req
        , Uri::ptr uri, int64_t timeout_ms) {
    
    Address::ptr addr = uri->createAddress();
    if (!addr) {
        return std::make_shared<HttpResult>((int)HttpResult::Status::INVALID_HOST, nullptr,
                 "invalid_host: " + uri->getHost());
    }

    Socket::ptr sock = Socket::CreateTCP(addr);
    if (!sock) {
        return std::make_shared<HttpResult>((int)HttpResult::Status::CREATE_SOCKET_ERROR, nullptr,
                "create socket failure: " + addr->toString() +
                ", errno=" + std::to_string(errno) + " errstr=" + std::string(strerror(errno)));
    }
    if (!sock->connect(addr, timeout_ms)) {
        return std::make_shared<HttpResult>((int)HttpResult::Status::CONNECT_FAILURE, nullptr,
                 "connect_failure: " + addr->toString());
    }
    sock->setRecvTimeout(timeout_ms);

    HttpConnection::ptr conn = std::make_shared<HttpConnection>(sock);
    int rt = conn->sendRequest(req);
    if (rt == 0) {
        return std::make_shared<HttpResult>((int)HttpResult::Status::SEND_CLOSE_BY_PEER, nullptr,
                 "send request cloesd by peer: " + addr->toString());
    } else if (rt < 0) {
        return std::make_shared<HttpResult>((int)HttpResult::Status::SEND_SOCKET_ERROR, nullptr,
                 "send request socket error, errno=" + std::to_string(errno)
                  + " errstr=" + std::string(strerror(errno)));
    }

    HttpResponse::ptr rsp = conn->recvResponse();
    if (!rsp) {
        return std::make_shared<HttpResult>((int)HttpResult::Status::TIMEOUT, nullptr,
                "recvResponse timeout: " + addr->toString() + " timeout_ms: " + std::to_string(timeout_ms));
    }

    return std::make_shared<HttpResult>((int)HttpResult::Status::OK, rsp, "OK");
}

// Get 请求
HttpResult::ptr HttpConnection::DoGet(const std::string& url
        , int64_t timeout_ms, const std::map<std::string, std::string>& headers
        , const std::string& body) {
    Uri::ptr uri = Uri::Create(url);
    if (!uri) {
        return std::make_shared<HttpResult>((int)HttpResult::Status::INVALID_URL, nullptr, 
                "invalid_url: " + url);
    }

    return DoGet(uri, timeout_ms, headers, body);
}

// Get 请求
HttpResult::ptr HttpConnection::DoGet(Uri::ptr uri
        , int64_t timeout_ms, const std::map<std::string, std::string>& headers
        , const std::string& body) {
    return DoRequest(HttpMethod::GET, uri, timeout_ms, headers, body);
}

// Post 请求
HttpResult::ptr HttpConnection::DoPost(const std::string& url
        , int64_t timeout_ms, const std::map<std::string, std::string>& headers
        , const std::string& body) {
    Uri::ptr uri = Uri::Create(url);
    if (!uri) {
        return std::make_shared<HttpResult>((int)HttpResult::Status::INVALID_URL, nullptr, 
                "invalid_url: " + url);
    }

    return DoPost(uri, timeout_ms, headers, body);
}

// Post 请求
 HttpResult::ptr HttpConnection::DoPost(Uri::ptr uri
        , int64_t timeout_ms, const std::map<std::string, std::string>& headers
        , const std::string& body) {
    return DoRequest(HttpMethod::POST, uri, timeout_ms, headers, body);
}

}
}