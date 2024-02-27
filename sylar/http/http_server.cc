#include "http_server.h"
#include "sylar/log.h"

namespace sylar {
namespace http {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

HttpServer::HttpServer(bool keepalive, sylar::IOManager*  worker, sylar::IOManager* accept_worker) 
        :TcpServer(worker, accept_worker)
        ,m_isKeepalive(keepalive) {
    m_dispatch.reset(new ServletDispatch);
    m_type = "http";
}

void HttpServer::setName(const std::string& v) {
    TcpServer::setName(v);
    m_dispatch->setDefault(std::make_shared<NotFoundServlet>(v));
}

void HttpServer::handleClient(Socket::ptr client)  {
    sylar::http::HttpSession::ptr session(new HttpSession(client));
    do {
        auto req = session->recvRequest();
        if (!req) {
            SYLAR_LOG_WARN(g_logger) << "recv http request failture, errno=" 
                    << errno << " errstr=" << strerror(errno)
                    << " client " << *client;
            break;
        }
        HttpResponse::ptr rsp(new HttpResponse(req->getVersion(), req->isClose() || !m_isKeepalive));
        m_dispatch->handle(req, rsp, session);

        // rsp->setBody("hello sylar");
        // SYLAR_LOG_INFO(g_logger) << "request: " << std::endl << *req;
        // SYLAR_LOG_INFO(g_logger) << "response: " << std::endl << *rsp;

        session->sendResponse(rsp);
    } while(m_isKeepalive);
    session->close();
}

}
}