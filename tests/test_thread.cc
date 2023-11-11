#include "sylar/sylar.h"
#include "sylar/config.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

int count = 0;
// sylar::RWMutex s_mutex;
sylar::Mutex s_mutex;


void fun1() {
    SYLAR_LOG_INFO(g_logger) << "name: " << sylar::Thread::GetName()
            << ", this.name: " << sylar::Thread::GetThis()->getName()
            << ", id: " << sylar::GetThreadId()
            << ", this.id: " << sylar::Thread::GetThis()->getId();
    
    // sleep(20);  // for top test

    for (int i = 0; i < 100000; ++i) {
        {
            // sylar::RWMutex::WriteLock lock(s_mutex);
            sylar::Mutex::Lock lock(s_mutex);
        
            ++count;
        }
    }

}

void fun2() {
    while (true) {
        SYLAR_LOG_INFO(g_logger) << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    }
}

void fun3() {
    while (true) {
        SYLAR_LOG_INFO(g_logger) << "==============================";
    }
}

void test_fun1() {
    SYLAR_LOG_INFO(g_logger) << "thread test begin";
    
    std::vector<sylar::Thread::ptr> thrs;
    for (int i = 0; i < 5; ++i) {
        sylar::Thread::ptr thr(new sylar::Thread(&fun1, "name_" + std::to_string(i)));
        thrs.push_back(thr);
    }

    for (int i = 0; i < 5; ++i) {
        thrs[i]->join();
    }

    SYLAR_LOG_INFO(g_logger) << "thread test end";

    SYLAR_LOG_INFO(g_logger) << "count = " << count;
}

void test_fun2_3() {
    // 测试 mutex logger，使用 FileLogAppender 测试
    SYLAR_LOG_INFO(g_logger) << "thread test begin";
    
    // root logger 改变
    YAML::Node root = YAML::LoadFile("/home/lambda/workspace/sylar/bin/conf/logger_mutex.yml");
    sylar::Config::loadFromYaml(root);          // 触发配置变化 -> 事件 -> logger 初始化

    std::vector<sylar::Thread::ptr> thrs;
    for (int i = 0; i < 2; ++i) {
        sylar::Thread::ptr thr(new sylar::Thread(&fun2, "name_" + std::to_string(i * 2)));
        sylar::Thread::ptr thr2(new sylar::Thread(&fun3, "name_" + std::to_string(i * 2 + 1)));

        thrs.push_back(thr);
        thrs.push_back(thr2);
    }

    for (size_t i = 0; i < thrs.size(); ++i) {
        thrs[i]->join();
    }

    SYLAR_LOG_INFO(g_logger) << "thread test end";
}
int main(int argc, char const *argv[]) {

    // test_fun1();

    test_fun2_3();
    return 0;
}
