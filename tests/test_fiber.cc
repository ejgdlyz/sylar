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

void test_fiber() {
    SYLAR_LOG_INFO(g_logger) << "main begin -1";

    {
        auto main_fiber = sylar::Fiber::GetThis();                   // 先创建一个 main 协程

        SYLAR_LOG_INFO(g_logger) << "main fiber_id = "
                << sylar::GetFiberId();
        
        sylar::Fiber::ptr fiber(new sylar::Fiber(run_in_fiber));    // 创建子协程
        fiber->swapIn();                                            // 切到 run_in_fiber
        
        SYLAR_LOG_INFO(g_logger) << "main after swapIn";
        
        fiber->swapIn();                                            
        
        SYLAR_LOG_INFO(g_logger) << "main after end";
        
        fiber->swapIn();    
    }
    SYLAR_LOG_INFO(g_logger) << "main after end2";

}

int main(int argc, char const *argv[]) {
    sylar::Thread::SetName("main");         // 设置线程名称
    
    std::vector<sylar::Thread::ptr> threads;
    for (int i = 0; i < 3; ++i) {
        
        threads.push_back(sylar::Thread::ptr(new sylar::Thread(&test_fiber, "name_" + std::to_string(i))));
    }
    
    for (auto& thread : threads) {
        thread->join();
    }
    return 0;
}
