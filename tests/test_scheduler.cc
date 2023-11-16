#include "sylar/sylar.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_fiber() {
    SYLAR_LOG_INFO(g_logger) << "test in fiber";
}

int main(int argc, char const *argv[]) {

    SYLAR_LOG_INFO(g_logger) << "main start";

    sylar::Scheduler sc;
    sc.start();
    sc.schedule(&test_fiber); 
    sc.stop();    
    SYLAR_LOG_INFO(g_logger) << "main over";
    return 0;
}
