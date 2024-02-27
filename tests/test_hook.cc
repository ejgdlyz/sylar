#include <sys/socket.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <arpa/inet.h>
#include <string.h>

#include "sylar/hook.h"
#include "sylar/iomanager.h"
#include "sylar/log.h"
#include "sylar/macro.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_sleep() {
    sylar::IOManager iom(1);    
    
    // 一共 sleep 5s ，只有一个线程，所以两个 sleep 同时进行

    iom.schedule([](){
        sleep(2);
        SYLAR_LOG_INFO(g_logger) << "sleep 2s";  // sleep 2s 
    });

    iom.schedule([](){
        sleep(3);
        SYLAR_LOG_INFO(g_logger) << "sleep 3s";  // sleep 3s 
    });

    SYLAR_LOG_INFO(g_logger) << "test_sleep";

}

int sock = -1;
void test_socket() {
    sock = socket(PF_INET, SOCK_STREAM, 0);
    SYLAR_ASSERT(sock >= 0);

    // fcntl(sock, F_SETFL, O_NONBLOCK); // 异步
    
    // int port = 9999;
    // const char* ip = "127.0.0.1"; //
    int port = 80;
    const char* ip = "110.242.68.4"; // baidu.com


    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &addr.sin_addr);
    addr.sin_port = htons(port);

    SYLAR_LOG_INFO(g_logger) << "connect begin";
    int rt = connect(sock, (const sockaddr*)&addr, sizeof(addr));
    SYLAR_LOG_INFO(g_logger) << "connect rt = " << rt << ", errno = " << errno;
    if (rt) { // fail
        return;
    }

    const char data[] = "GET / HTTP/1.0\r\n\r\n";
    rt = send(sock, data, sizeof(data), 0);
    SYLAR_LOG_INFO(g_logger) << "send rt = " << rt << ", errno = " << errno;

    if (rt <= 0) {
        return;
    }

    std::string buf;
    buf.resize(4096);

    rt = recv(sock, &buf[0], buf.size(), 0);
    SYLAR_LOG_INFO(g_logger) << "recv rt = " << rt << ", errno = " << errno;

    if (rt <= 0) {
        return;
    }

    buf.resize(rt);
    SYLAR_LOG_INFO(g_logger) << buf;

}

int main(int argc, char const *argv[])
{
    // test_sleep();
    // test_socket();  // 不是 iomanager 线程，不会使用 hook
    sylar::IOManager iom;
    iom.schedule(test_socket);
    return 0;
}
