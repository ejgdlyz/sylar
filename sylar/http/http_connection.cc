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

}
}