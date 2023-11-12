#include "sylar/sylar.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void run_in_fiber() {
    SYLAR_LOG_INFO(g_logger) << "run_in_fiber begin: fiber_id = " 
            << sylar::GetFiberId(); 
    sylar::Fiber::YieldToHold();                                // 返回 main:fiber->swapIn();
    SYLAR_LOG_INFO(g_logger) << "run_in_fiber end: fiber_id = " 
            << sylar::GetFiberId();
    sylar::Fiber::YieldToHold();                                
}

int main(int argc, char const *argv[])
{
    SYLAR_LOG_INFO(g_logger) << "main begin";
    sylar::Fiber::GetThis();                                    // 先创建一个 main 协程
    SYLAR_LOG_INFO(g_logger) << "main fiber_id = " << sylar::GetFiberId();
    sylar::Fiber::ptr fiber(new sylar::Fiber(run_in_fiber));    // 创建协程
    fiber->swapIn();                                            // 切到 run_in_fiber
    SYLAR_LOG_INFO(g_logger) << "main after swapIn";
    fiber->swapIn();                                            
    SYLAR_LOG_INFO(g_logger) << "main end";

    return 0;
}
