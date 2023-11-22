#include "sylar/hook.h"
#include "sylar/iomanager.h"
#include "sylar/log.h"

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

int main(int argc, char const *argv[])
{
    test_sleep();
    return 0;
}
