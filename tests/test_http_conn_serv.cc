#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    int listenfd;
    sockaddr_in serv_addr;  // p41
    sockaddr_in clnt_addr;

    listenfd = socket(PF_INET, SOCK_STREAM, 0);  

    memset(&serv_addr, 0, sizeof(serv_addr));  
    serv_addr.sin_family = AF_INET;            
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    serv_addr.sin_port = htons(atoi(argv[1]));  

    int rt = 0;
    rt = bind(listenfd, (const sockaddr*) &serv_addr, sizeof(serv_addr));
    assert(rt != -1);
    
   
    rt = listen(listenfd, 5);
    assert(rt != -1);
    
    int connfd;
    socklen_t clnt_addr_size;
    clnt_addr_size = sizeof(clnt_addr);
    connfd = accept(listenfd, (sockaddr*) &clnt_addr, &clnt_addr_size);
    assert(connfd >= 0);
    
    int ret = 0;
    
    // char data[1024];
    // memset(&data, 0, sizeof(data));
    
    // ret = read(connfd, data, 200);
    // data[ret] = '\0';
    // std::cout << "read ret = " << ret << std::endl 
    //     << " errno = " << errno << ", errstr = " << strerror(errno) << std::endl;

    std::string rsp = "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/plain\r\n"
                        "Transfer-Encoding: chunked\r\n\r\n"
                        "7\r\n"
                        "Mozilla\r\n"
                        "9\r\n"
                        "Developer\r\n"
                        "7\r\n"
                        "Network\r\n"
                        "0\r\n" 
                        "\r\n";

    ret = send(connfd, &rsp[0], rsp.size(), 0);
    if (ret <= 0) {
        std::cout << "send error" << std::endl;
    }
    close(connfd);
    close(listenfd);

    return 0;
}