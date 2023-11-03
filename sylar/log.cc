#include "log.h"
#include <iostream>
#include <functional>
#include <time.h>
#include <string.h>

namespace sylar
{

const char* LogLevel::ToString(LogLevel::Level level) {
    switch (level) {
#define XX(name) \
    case LogLevel::name: \
        return #name;  \
        break;
    XX(DEBUG);
    XX(INFO);
    XX(WARN);
    XX(ERROR);
    XX(FATAL);
#undef XX
    default:
        return "UNKNOWN";
    }
    return "UNKNOWN";
}

LogEventWrap::LogEventWrap(LogEvent::ptr event) 
    : m_event(event) {

}

LogEventWrap::~LogEventWrap() {
    m_event->getLogger()->log(m_event->getLevel(), m_event);  // 将自己写进 log 
}

std::stringstream& LogEventWrap::getSS() {
    return m_event->getSS();
}

void LogEvent::format(const char* fmt, ...) {
    va_list al;
    va_start(al, fmt);
    format(fmt, al);
    va_end(al);
}

void LogEvent::format(const char* fmt, va_list va) {
    char* buf = nullptr;
    int len = vasprintf(&buf, fmt, va);  // 根据 fmt，al 分配内存, 然后 序列化
    if (len != -1) {
        m_ss << std::string(buf, len);  // 写到 stringstream
    }
    free(buf);
}

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
        os << event->getThreadId();
    }
};

class FiberIdFormatterItem : public LogFormatter::FormatterItem {
public:
    FiberIdFormatterItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getFiberId();
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

class TabFormatterItem : public LogFormatter::FormatterItem {
public:
    TabFormatterItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << "\t";
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


LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char* file, int32_t line, uint32_t elpase, 
                        uint32_t thread_id, uint32_t fiber_id, uint64_t time)
    : m_file(file)
    ,m_line(line)
    ,m_elpase(elpase)
    ,m_threadId(thread_id)
    ,m_fiberId(fiber_id)
    ,m_time(time)
    ,m_logger(logger)
    ,m_level(level) {

 }
Logger::Logger(const std::string& name, LogLevel::Level level) 
    :m_name(name), m_level(level) {
        m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));    // default 格式, LogFormatter::init() 解析该字符串
}

void Logger::addAppender(LogAppender::ptr appender) {
    if (!appender->getFormatter()) {
        appender->setFormatter(m_formatter);  // 保证每个都有 formatter(default)
    }
    m_appenders.push_back(appender);
}
void Logger::delAppender(LogAppender::ptr appender) {
    for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it)
    {
        if (*it == appender)
        {
            m_appenders.erase(it); 
            break;
        }
    }
}

void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
    if (level >= m_level)
    {
        auto self = shared_from_this();
        for (auto& appender :  m_appenders) {
            appender->log(self, level, event);  // 遍历 LogAppender::log, like StdoutApppender->log()...
        }
    }
}


// 输出 debug 级别日志
void Logger::debug(LogEvent::ptr event) {
    log(LogLevel::DEBUG, event);
}
void Logger::infor(LogEvent::ptr event) {
    log(LogLevel::INFO, event);
}

void Logger::warn(LogEvent::ptr event) {
    log(LogLevel::WARN, event);

}
void Logger::error(LogEvent::ptr event) {
    log(LogLevel::ERROR, event);

}
void Logger::fatal(LogEvent::ptr event) {
    log(LogLevel::FATAL, event);

}


FileLogAppender::FileLogAppender(const std::string& filename)
    : m_filename(filename) {
    reopen();
}

void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    if (level >= m_level){
        m_filestream << m_formatter->format(logger, level, event);
    }
}

bool FileLogAppender::reopen() {
    if (m_filestream) {
        m_filestream.close();
    }
    m_filestream.open(m_filename);
    return !!m_filestream;
}

void StdoutLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    if (level >= m_level) {
        std::cout << m_formatter->format(logger, level, event);  // 得到基类 LogFormatter::format() 返回的 string
    }
}


LogFormatter::LogFormatter(const std::string& pattern) 
    :m_pattern(pattern) {
    init();  // 解析 pattern
}

std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    std::stringstream ss;
    for(auto& formatter_item : m_items) {
        formatter_item->format(ss, logger, level, event);  // 解析后的各个字符对应的特定类内容，输出到字符流中, like DateTimeFormatterItem、StringFormatterItem 
    }
    return ss.str();
}

// %xxx、%xxx{xxx}、 %%。 其余认为是非法的 
void LogFormatter::init() {
    // 解析 pattern: (str, format, type) 三元组格式
    std::vector<std::tuple<std::string, std::string, int> > vec;
    std::string nstr;
    for (size_t i = 0; i < m_pattern.size(); ++i) {
        if (m_pattern[i] != '%') {       // != %
            nstr.append(1, m_pattern[i]);
            continue;
        }

        if ((i + 1) < m_pattern.size() && m_pattern[i + 1] == '%') {  // %%
            nstr.append(1, '%');
            continue;
        }

        size_t n = i + 1;   // %xxx, %<>, %{}
        int fmt_status = 0;
        size_t fmt_begin = 0;

        std::string str;
        std::string fmt;

        while (n < m_pattern.size()) {  
            if (!fmt_status && (!isalpha(m_pattern[n]) && m_pattern[n] != '{' && m_pattern[n] != '}')) {
                str = m_pattern.substr(i + 1, n - i - 1);
                break;
            }
            if (fmt_status == 0) {
                if (m_pattern[n] == '{') {  // %{...}
                    str = m_pattern.substr(i + 1, n - i - 1);
                    // std::cout << "*" << str << std::endl; 
                    fmt_status = 1;   // 解析格式
                    fmt_begin = n;
                    ++n;
                    continue;
                }
            } else if (fmt_status == 1) {   // %{...}
                if (m_pattern[n] == '}') {
                    fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                    // std::cout << "#" << fmt << std::endl; 
                    fmt_status = 0;
                    ++n;
                    break;
                }
            }

            ++n;
            if (n == m_pattern.size()) {
                if (str.empty()) {
                    str = m_pattern.substr(i + 1);
                }
            }
        }

        if (fmt_status == 0) {
            if (!nstr.empty()) {
                vec.push_back(std::make_tuple(nstr, std::string(), 0));
                nstr.clear();
            }
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n - 1;
        } else if (fmt_status == 1) {
            // std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
            // m_error = true;
            vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
        }

    }
    
    if (!nstr.empty()) {
        vec.push_back(std::make_tuple(nstr, "", 0));
    }

   static std::map<std::string, std::function<FormatterItem::ptr(const std::string& str)> > s_format_items = {
#define XX(str, C) \
        {#str, [](const std::string& fmt) { return FormatterItem::ptr( new C(fmt));}}

        XX(m, MessageFormatterItem),
        XX(p, LevelFormatterItem), 
        XX(r, ElapseFormatterItem), 
        XX(c, NameFormatterItem), 
        XX(t, ThreadIdFormatterItem), 
        XX(n, NewLineFormatterItem), 
        XX(d, DateTimeFormatterItem), 
        XX(f, FilenameFormatterItem), 
        XX(l, LineFormatterItem), 
        XX(T, TabFormatterItem), 
        XX(F, FiberIdFormatterItem), 
#undef XX
    };

    for (auto& tuple_v : vec) {
        if (std::get<2>(tuple_v) == 0) {            // 非定义字符 like "] <"，用一个正常字符串类接收
            m_items.push_back(FormatterItem::ptr(new StringFormatterItem(std::get<0>(tuple_v))));  
        } else {                              // 自定义字符 like %m, %p, ....,                             
            auto it = s_format_items.find(std::get<0>(tuple_v));
            if (it == s_format_items.end()) {
                m_items.push_back(FormatterItem::ptr(new StringFormatterItem("<<error_format %" + std::get<0>(tuple_v) + ">>")));
                // m_error = true;
            
            } else {
                m_items.push_back(it->second(std::get<1>(tuple_v)));  // like,  "m" 对应的 MessageFormatterItem 放入 m_items 
            }
        }

        // std::cout << "{" << std::get<0>(tuple_v) << "} - {" << std::get<1>(tuple_v) << "} - {" << std::get<2>(tuple_v) << "}" << std::endl;
    }
    
    // std::cout << m_items.size() << std::endl;   
    /*
    %m -- 消息体
    %p -- level
    %r -- 启动后的时间
    %c -- 日志名称
    %t -- 线程 id
    %n -- 回车换行
    %d -- 时间
    %f -- 文件名
    %l -- 行号
    */

}

LoggerManager::LoggerManager() {
    m_root.reset(new Logger);  // 默认logger
    m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));  // 默认 appender
}
Logger::ptr LoggerManager::getLogger(const std::string& name) {
    auto it = m_loggers.find(name);
    return it == m_loggers.end() ? m_root : it->second;
}

}