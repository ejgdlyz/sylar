#ifndef __SYLAR_TCP_SERVER__
#define __SYLAR_TCP_SERVER__

#include <memory>
#include <functional>
#include <vector>
#include <string>
#include "iomanager.h"
#include "socket.h"
#include "address.h"
#include "noncopyable.h"

namespace sylar {

class TcpServer : public std::enable_shared_from_this<TcpServer>, NonCopyable {
public:
    typedef std::shared_ptr<TcpServer> ptr;
    TcpServer(sylar::IOManager* work = sylar::IOManager::GetThis(), 
        sylar::IOManager* accept_work = sylar::IOManager::GetThis());
    virtual ~TcpServer();

    virtual bool bind(sylar::Address::ptr addr);                // 监听地址
    virtual bool bind(const std::vector<Address::ptr>& addrs, std::vector<Address::ptr>& failed_addrs);
    virtual bool start();  
    virtual void stop();

    uint64_t getRecvTimeout() const { return m_recvTimeout;}
    std::string getName() const { return m_name;}
    void setRecvTimeout(uint64_t v) { m_recvTimeout = v;}
    void setName(const std::string& v) { m_name = v;}
    bool isStop() const { return m_isStop;}

protected:
    virtual void handleClient(Socket::ptr client);  // 每 accpet 一次，执行一次这个回调函数
    virtual void startAccept(Socket::ptr sock);
private:
    std::vector<Socket::ptr> m_socks;       //
    IOManager* m_worker;                    // 工作线程池，执行 hanleClient()
    IOManager* m_acceptWork;                // accept() 线程池 
    uint64_t m_recvTimeout;
    std::string m_name;
    bool m_isStop;
};


}


#endif  // __SYLAR_TCP_SERVER__