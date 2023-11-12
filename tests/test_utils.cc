#include <assert.h>
#include "sylar/sylar.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_assert() {
    // SYLAR_LOG_INFO(g_logger) << sylar::BacktraceToString(10, 0, "    ");
    // SYLAR_LOG_INFO(g_logger) << sylar::BacktraceToString(10, 2, "    ");  // 跳过前两层
    SYLAR_LOG_INFO(g_logger) << sylar::BacktraceToString(10);  // 或者，使用默认值(skip 2)

    // SYLAR_ASSERT(false);
    
    SYLAR_ASSERT2(0 == 1, "testing for macro with two parameters: SYLAR_ASSERT2");

}
int main(int argc, char const *argv[])
{
    test_assert();
    return 0;
}
