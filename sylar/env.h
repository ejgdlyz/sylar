#ifndef __SYLAR_ENV_H__
#define __SYLAR_ENV_H__

#include <map>
#include <vector>
#include <string>
#include "sylar/singleton.h"
#include "sylar/thread.h"

namespace sylar {

class Env {
public:
    typedef RWMutex RWMutexType;
    bool init(int argc, char **argv);
    
    void add(const std::string &key, const std::string& val);
    bool has(const std::string& key);
    void del(const std::string& key);
    std::string get(const std::string& key, const std::string& default_val = "");

    void addHelper(const std::string& key, const std::string& desc);
    void removeHelper(const std::string& key);
    void printHelper();
private:
    RWMutexType m_mutex;
    std::map<std::string, std::string> m_args;                  // 参数名: 参数值
    std::vector<std::pair<std::string, std::string>> m_helpers;  // 保存参数描述
    
    std::string m_program;
};

typedef sylar::Singleton<Env> EnvMgr;
}

#endif