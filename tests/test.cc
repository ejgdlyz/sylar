#include <iostream>
#include "sylar/log.h"

int main(int argc, char** argv) {
    sylar::Logger::ptr logger(new sylar::Logger);
    
    logger->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender));
    
    sylar::LogEvent::ptr event(new sylar::LogEvent(__FILE__, __LINE__, 0, 1, 2, time(0)));
    logger->log(sylar::LogLevel::DEBUG, event);



    // sylar::FileLogAppender::ptr file_appender(new sylar::FileLogAppender("./log.txt"));
    // // sylar::LogFormatter::ptr fmt(new sylar::LogFormatter("%d%T%p%T%m%n"));
    // sylar::LogFormatter::ptr fmt(new sylar::LogFormatter("%d [%p] <%f %l> %m %n"));

    // file_appender->setFormatter(fmt);
    // // file_appender->setLevel(sylar::LogLevel::ERROR);

    // logger->addAppender(file_appender);
    // logger->log(sylar::LogLevel::ERROR, event);

    std::cout << "hello sylar log" << std::endl;

    return 0;
}