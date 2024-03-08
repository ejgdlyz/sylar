#include "sylar/sylar.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_fiber() {
    static int s_cnt = 5;

    SYLAR_LOG_INFO(g_logger) << "test in fiber, s_cnt = " << s_cnt;

    // sleep(1);
    sleep_f(1);  // 调用未被 hooked，即原始的 sleep()。 不然会报段错误
    if (--s_cnt >= 0) {
        sylar::Scheduler::GetThis()->schedule(&test_fiber);  // 每秒循环一次，一个协程执行完毕后又添加新任务到调度器
        // sylar::Scheduler::GetThis()->schedule(&test_fiber, sylar::GetThreadId());  // 指定执行线程 GetThreadId，不能切换

    }
}

int main(int argc, char const *argv[]) {

    SYLAR_LOG_INFO(g_logger) << "main start";
    // sylar::Scheduler sc(1, true, "test");
    sylar::Scheduler sc(3, false, "test");  // true 使用当前线程，false 为不使用当前线程，这将创建 3 个线程
    sc.start();
    sleep(2);
    SYLAR_LOG_INFO(g_logger) << "schedule";
    sc.schedule(&test_fiber); 
    sc.stop();    
    SYLAR_LOG_INFO(g_logger) << "main over";
    return 0;
}
