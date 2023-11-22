#include "sylar/sylar.h"
#include "sylar/macro.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

int main(int argc, char const *argv[])
{
    int serv_sock;

    sockaddr_in serv_addr;  // p41
    sockaddr_in clnt_addr;

    // char message[] = "Hello World!";

    if (argc != 2)
    {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);  
    SYLAR_ASSERT(serv_sock >= 0);

    memset(&serv_addr, 0, sizeof(serv_addr));  
    serv_addr.sin_family = AF_INET;            
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    serv_addr.sin_port = htons(atoi(argv[1]));  

    int rt = 0;
    rt = bind(serv_sock, (const sockaddr*) &serv_addr, sizeof(serv_addr));
    std::cout << "rt = " << rt << ", "<< errno << " -> " << strerror(errno) << std::endl;
    SYLAR_ASSERT(rt != -1);
    
   
    rt = listen(serv_sock, 5);
    SYLAR_ASSERT(rt != -1);
    SYLAR_LOG_INFO(g_logger) << "listen...";
    
    int clnt_sock;
    socklen_t clnt_addr_size;
    clnt_addr_size = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (sockaddr*) &clnt_addr, &clnt_addr_size);
    SYLAR_ASSERT(clnt_sock >= 0);
    
    // const char* msg = "T";
    // write(clnt_sock, msg, sizeof(msg));

    // case1：延时关闭连接，
    int cnt = 10;
    while (--cnt >= 0) {
        SYLAR_LOG_INFO(g_logger) << "cnt = " << cnt;

        sleep(2);
    }
    
    // case 2: 立即关闭

    // // case 3: not exists
    // int t = 0;
    // std::cin >> t; 
    // std::cout << "case 3: t = " << t << std::endl;

    // 关闭套接字
    close(clnt_sock);
    close(serv_sock);

    return 0;
}