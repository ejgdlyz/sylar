#include "sylar/sylar.h"
#include "sylar/iomanager.h"
#include "sylar/macro.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <arpa/inet.h>
#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

int sock = 0;
void test_fiber() {
    SYLAR_LOG_INFO(g_logger) << "test_fiber sock = " << sock;

    sock = socket(PF_INET, SOCK_STREAM, 0);
    SYLAR_ASSERT(sock >= 0);

    // setsockopt(sock, SOL_SOCKET, );
    fcntl(sock, F_SETFL, O_NONBLOCK); // 异步
    
    int port = 9999;
    const char* ip = "127.0.0.1"; // baidu.com
    // const char* ip = "110.242.68.4"; // baidu.com


    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    // addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // addr.sin_addr.s_addr = inet_addr(ip);
    inet_pton(AF_INET, ip, &addr.sin_addr);
    addr.sin_port = htons(port);

    int rt = connect(sock, (const sockaddr*)&addr, sizeof(addr));
    SYLAR_ASSERT(rt);
    if (errno == EINPROGRESS) {
        SYLAR_LOG_INFO(g_logger) << "addEvent errno = " << errno << ", " << strerror(errno);
        
        sylar::IOManager::GetThis()->addEvent(sock, sylar::IOManager::READ, [](){
            SYLAR_LOG_INFO(g_logger) << "read callback";
        });
        
        sylar::IOManager::GetThis()->addEvent(sock, sylar::IOManager::WRITE, [](){
            SYLAR_LOG_INFO(g_logger) << "write callback";

            sylar::IOManager::GetThis()->cancelEvent(sock, sylar::IOManager::READ);
            close(sock);

        });

    } else {
        SYLAR_LOG_INFO(g_logger) << "else " << errno << " " << strerror(errno);
    }

}

void test01() {
    std::cout << "EPOLLIN = " << EPOLLIN 
            << ", EPOLLOUT = " << EPOLLOUT << std::endl;

    sylar::IOManager iom;
    // sylar::IOManager iom(2, false);
    iom.schedule(&test_fiber);

}

sylar::Timer::ptr s_timer;
void test_timer() {
    sylar::IOManager iom(2);
    s_timer = iom.addTimer(1000, [](){
        SYLAR_LOG_INFO(g_logger) << "hello timer";
        static int i = 0;
        if (++i == 3) {
            // s_timer->cancel();
            s_timer->reset(2000, true);
        }
    }, true);
}

int main(int argc, char const *argv[])
{
    // test01();
    test_timer();    
    return 0;
}
