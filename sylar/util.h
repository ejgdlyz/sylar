#ifndef __SYLAR_UTIL_H__
#define __SYLAR_UTIL_H__

#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <string>
#include "sylar/util/hash_util.h"

namespace sylar {

pid_t GetThreadId();

uint32_t GetFiberId();

void Backtrace(std::vector<std::string>& bt, int size = 64, int skip = 1); 

std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = "");

// 时间 ms
uint64_t GetCurretMS();
// 时间 us
uint64_t GetCurretUS();

std::string Time2Str(time_t ts = time(0), const std::string& format = "%Y-%m-%d %H:%M:%S");

class FSUtil {
public:
    // 返回 path 下所有的后缀名为 subfix 的 所有文件名
    static void ListAllFile(std::vector<std::string>& files, const std::string& path, const std::string& subfix);
    static bool Mkdir(const std::string& dirname);
    static bool IsRunningPidfile(const std::string& pidfile);
};

}


#endif