#ifndef __SYLAR_LOG_H
#define __SYLAR_LOG_H
#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <sstream>
#include <fstream>
#include <vector>

namespace sylar {

class Logger;
class LogFormatter;

// 日志事件
class LogEvent {
public:
    typedef std::shared_ptr<LogEvent> ptr;
    LogEvent(const char* file, int32_t line, uint32_t elpase, uint32_t thread_id, uint32_t fiber_id, uint64_t time);

    const char* getFile() const { return m_file;}
    int32_t getLine() const { return m_line;}
    uint32_t getElapse() const { return m_elpase;}
    uint32_t getThreadId() const { return m_threadId;}
    uint32_t getFiberId() const { return m_fiberId;}
    uint64_t getTime() const { return m_time;}
    std::string getContent() const { return m_ss.str();}
    
    std::stringstream& getSS() {return m_ss;}
private:
    const char* m_file = nullptr;  // 文件名
    int32_t m_line = 0;            // 行号
    uint32_t m_elpase = 0;         // 程序运行的毫秒数
    uint32_t m_threadId = 0;       // 线程 id
    uint32_t m_fiberId = 0;        // 协程 id
    uint64_t m_time;               // 时间戳
    std::stringstream m_ss;        // 内容
};

// 日志级别
class LogLevel {
public:
    enum Level {  // 日志级别
        UNKNOWN = 0, 
        DEBUG = 1, 
        INFO = 2, 
        WARN = 3, 
        ERROR = 4, 
        FATAL = 5
    };
    static const char* ToString(LogLevel::Level level);
};

// 日志格式器
class LogFormatter {
public:
    typedef std::shared_ptr<LogFormatter> ptr;
    LogFormatter(const std::string& pattern);  // 根据 pattern 来解析 FormatterItem

    // %t   %thread_id %m%n ...
    std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);   // 将 LogEvent format 为一个 string 提供给 LogAppender 输出
public:
    class FormatterItem {
    public:
        typedef std::shared_ptr<FormatterItem> ptr;
        virtual ~FormatterItem () {}
        virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    };

    void init();  // pattern 解析
private:
    std::string m_pattern;
    std::vector<FormatterItem::ptr> m_items;
};

// 日志输出地
class LogAppender {
public:
    typedef std::shared_ptr<LogAppender> ptr;
    virtual ~LogAppender() {}

    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0; 

    void setFormatter(LogFormatter::ptr formatter) {m_formatter = formatter;}
    LogFormatter::ptr getFormatter() const {return m_formatter;}

    void setLevel(LogLevel::Level level) {m_level = level;}
protected:
    LogLevel::Level m_level = LogLevel::DEBUG;
    LogFormatter::ptr m_formatter;
};


// 日志输出器
class Logger : public std::enable_shared_from_this<Logger> {
public:
    typedef std::shared_ptr<Logger> ptr;
    
    Logger(const std::string& name = "root");

    void log(LogLevel::Level level, LogEvent::ptr event); 

    void debug(LogEvent::ptr event);   // 输出 debug 级别日志
    void infor(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
    
    void setLevel(LogLevel::Level level) {m_level = level;}
    LogLevel::Level getLevel() const {return m_level;}

    const std::string& getName() const { return m_name;}
private:
    std::string m_name;                         // 日志名称
    LogLevel::Level m_level;                    // 日志级别
    std::list<LogAppender::ptr> m_appenders;    // Appender 集合
    LogFormatter::ptr m_formatter;              // 直接使用 formatter
};

// 输出到控制台的 Appender
class StdoutLogAppender : public LogAppender{
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override;
private:
};

// 输出到文件的 Appender
class FileLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string& filename);
    void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override; 
    bool reopen();  // 重新打开文件，成功返回 true
private:
    std::string m_filename;
    std::ofstream m_filestream;
};

class MessageFormatterItem : public LogFormatter::FormatterItem {
public:
    MessageFormatterItem(const std::string& str = "") {} 
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getContent();
    }
};

class LevelFormatterItem : public LogFormatter::FormatterItem {
public:
    LevelFormatterItem(const std::string& str = "") {} 
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << LogLevel::ToString(level);
    }
};

class ElapseFormatterItem : public LogFormatter::FormatterItem {
public:
    ElapseFormatterItem(const std::string& str = "") {} 
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getElapse();
    }
};

class NameFormatterItem : public LogFormatter::FormatterItem {
public:
    NameFormatterItem(const std::string& str = "") {} 
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << logger->getName();
    }
};

class ThreadIdFormatterItem : public LogFormatter::FormatterItem {
public:
    ThreadIdFormatterItem(const std::string& str = "") {} 
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << logger->getName();
    }
};

class FiberIdFormatterItem : public LogFormatter::FormatterItem {
public:
    FiberIdFormatterItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << logger->getName();
    }
};

class DateTimeFormatterItem : public LogFormatter::FormatterItem {
public:
    DateTimeFormatterItem (const std::string& format = "%Y-%m-%d %H:%M:%S")
        :m_formatter(format) 
    {
        if (m_formatter.empty()) {
            m_formatter = "%Y-%m-%d %H:%M:%S";
        }
    }
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        struct tm tm;
        time_t time = event->getTime();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), m_formatter.c_str(), &tm);
        os << buf;
    }

private:
    std::string m_formatter;
};

class FilenameFormatterItem : public LogFormatter::FormatterItem {
public:
    FilenameFormatterItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getFile();
    }
};

class LineFormatterItem : public LogFormatter::FormatterItem {
public:
    LineFormatterItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getLine();
    }
};

class NewLineFormatterItem : public LogFormatter::FormatterItem {
public:
    NewLineFormatterItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << std::endl;
    }
};

class StringFormatterItem : public LogFormatter::FormatterItem {
public:
    StringFormatterItem (const std::string& str = "") 
        : m_string(str)
    {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << m_string;
    }

private:
    std::string m_string;
};

}
#endif