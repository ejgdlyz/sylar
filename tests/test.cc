#include <iostream>
#include <thread>
#include "sylar/log.h"
#include "sylar/util.h"

int main(int argc, char** argv) {
    sylar::Logger::ptr logger(new sylar::Logger);
    
    logger->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender));
    
    // std::thread::id threadId = std::this_thread::get_id();
    // std::hash<std::thread::id> hasher;
    // uint32_t threadIdHash = static_cast<uint32_t>(hasher(threadId));
    // sylar::LogEvent::ptr event(new sylar::LogEvent(__FILE__, __LINE__, 0, threadIdHash, 2, time(0)));

    // sylar::LogEvent::ptr event(new sylar::LogEvent(logger, sylar::LogLevel::DEBUG, 
    //     __FILE__, __LINE__, 0, sylar::GetThreadId(), sylar::GetFiberId(), time(0)));

    // event->getSS() << "Hello Sylar Log";  // log 内容
    
    // logger->log(sylar::LogLevel::DEBUG, event);


    // FileLogAppender test
    sylar::FileLogAppender::ptr file_appender(new sylar::FileLogAppender("./log.txt"));

    // 自定义 LogFormatter 
    sylar::LogFormatter::ptr fmt(new sylar::LogFormatter("%d%T%p%T%m%n"));
    file_appender->setFormatter(fmt);
    file_appender->setLevel(sylar::LogLevel::ERROR);  // 只将 ERROR 级别的日志写入文件

    logger->addAppender(file_appender);

    std::cout << "hello sylar log" << std::endl;

    // 流式宏测试
    SYLAR_LOG_INFO(logger) << "sstream INFO test micro";    // 创建临时对象 LogEventWrap(), 析构时自动写入。下同
    SYLAR_LOG_ERROR(logger) << "sstream ERROR test micro";

    // format 宏测试
    SYLAR_LOG_FMT_ERROR(logger, "fmt ERROR: test macro  %s", "aa");  // 同上

    auto logger2 = sylar::LoggerMgr::GetInstance()->getLogger("xx");
    SYLAR_LOG_INFO(logger2) << "XXX";
    return 0;
}