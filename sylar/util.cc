#include <execinfo.h>
#include <sys/time.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include "util.h"
#include "log.h"
#include "fiber.h"

namespace sylar {
static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");  // 系统日志统一用 system

pid_t GetThreadId() {
    return syscall(SYS_gettid);
}

uint32_t GetFiberId() {
    return sylar::Fiber::GetFiberId();
}

void Backtrace(std::vector<std::string>& bt, int size, int skip) {
    void** array = (void** )malloc(sizeof(void*) * size);
    size_t s = ::backtrace(array, size);

    char** strings = backtrace_symbols(array, s);

    if (strings == NULL) {
        SYLAR_LOG_ERROR(g_logger) << "backtrace_symbols error";
        return;
    }

    for(size_t i = skip; i < s; ++i) {
        bt.push_back(strings[i]);
    }
    
    free(strings);
    free(array);
}

std::string BacktraceToString(int size, int skip, const std::string& prefix) {
    std::vector<std::string> bt;
    Backtrace(bt, size, skip);
    std::stringstream ss;

    for(size_t i = 0; i < bt.size(); ++i) {
        ss << prefix << bt[i] << std::endl;
    }

    return ss.str();
}

// 时间 ms
uint64_t GetCurretMS() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000ul + tv.tv_usec / 1000;
}

// 时间 us
uint64_t GetCurretUS() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 * 1000ul + tv.tv_usec;
}

// 时间戳转为字符串
std::string Time2Str(time_t ts, const std::string& format) {
    struct tm tm;
    localtime_r(&ts, &tm);
    char buf[64];
    strftime(buf, sizeof(buf), format.c_str(), &tm);
    return buf;
}

void FSUtil::ListAllFile(std::vector<std::string>& files, 
        const std::string& path, const std::string& subfix) {
    if (access(path.c_str(), 0) != 0) {
        // 文件路径不存在
        return;
    }

    DIR* dir = opendir(path.c_str());
    if (dir == nullptr) {
        return;
    }
    struct dirent* dt = nullptr;
    while ((dt = readdir(dir)) != nullptr) {
        if (dt->d_type == DT_DIR) {
            // 文件夹，继续读
            if (!strcmp(dt->d_name, ".") || !strcmp(dt->d_name, "..")) {
                continue;
            }
            ListAllFile(files, path + "/" + dt->d_name, subfix);
        } else if(dt->d_type == DT_REG) {
            // 正常的文件
            std::string filename(dt->d_name);
            if (subfix.empty()) {
                files.push_back(path + "/" + filename);
            } else {
                if (filename.size() < subfix.size()) {
                    continue;
                }
                if (filename.substr(filename.length() - subfix.size()) == subfix) {
                    files.push_back(path + "/" + filename);
                }
            }
        }
    }
    closedir(dir);
}

}