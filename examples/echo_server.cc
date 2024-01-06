#include "sylar/tcp_server.h"
#include "sylar/iomanager.h"
#include "sylar/log.h"
#include "sylar/log.h"
#include "sylar/socket.h"
#include "sylar/bytearray.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

class EchoServer : public sylar::TcpServer {
public:
    // typedef std::shared_ptr<EchoServer> ptr;
    EchoServer(int type);
    void handleClient(sylar::Socket::ptr client);
private:
    int m_type = 1;  // 1: 文本形式, 0: 二进制形式
};

EchoServer::EchoServer(int type) 
    :m_type(type){
}

void EchoServer::handleClient(sylar::Socket::ptr client) {
    SYLAR_LOG_INFO(g_logger) << "EchoServer handleClinet: " << *client;
    sylar::ByteArray::ptr ba(new sylar::ByteArray);
    
    while (true) {
        ba->clear();
        std::vector<iovec> iovs;
        ba->getWriteBuffers(iovs, 1024);

        int rt = client->recv(&iovs[0], iovs.size());
        if (rt == 0) {
            SYLAR_LOG_INFO(g_logger) << "client closed: " << *client;
            break;
        } else if (rt < 0) {
            SYLAR_LOG_INFO(g_logger) << "client error rt=" << rt 
                << " errno=" << errno << " errstr=" << strerror(errno);
            break;
        } else {
            ba->setPosition(ba->getPosition() + rt);

            ba->setPosition(0);  // for toString()
            // SYLAR_LOG_INFO(g_logger) << "recv rt=" << rt << " data=" << std::string((char*)iovs[0].iov_base, rt);
            if (m_type == 1) {
                // text 形式
                // SYLAR_LOG_INFO(g_logger) << ba->toString();
                std::cout << ba->toString();
            } else {
                // binary 形式
                // SYLAR_LOG_INFO(g_logger) << ba->toHexString();
                std::cout << ba->toHexString();
            }
            std::cout.flush();
        }
    }
}

int type = 1;

void run() {
    EchoServer::ptr es(new EchoServer(type));  // EchoServer::ptr -> std::shared_ptr<TcpServer>
    auto addr = sylar::Address::LookupAny("0.0.0.0:8020");
    while (!es->bind(addr)) {
        sleep(2);
    }

    es->start();
}

int main(int argc, char const *argv[]) {
    // echo_server -t 
    if (argc < 2) {
        SYLAR_LOG_INFO(g_logger) << "used as [" << argv[0] << " -t] or [" << argv[0] << " -b]";
        return 0;
    }

    if (strcmp(argv[1], "-b") == 0) {
        type = 0;
    }

    sylar::IOManager iom(2);  // 1 个线程方便 debug
    iom.schedule(run);
    return 0;
}
