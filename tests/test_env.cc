#include <iostream>
#include <fstream>
#include <unistd.h>
#include "sylar/env.h"

// 通过全局变量构造函数，实现在 main() 之前解析参数 
struct A {
    A() {
        std::ifstream ifs("/proc/" + std::to_string(getpid()) + "/cmdline", std::ios::binary);
        std::string content;
        content.resize(4096);

        ifs.read(&content[0], content.size());
        content.resize(ifs.gcount());
        for (int i = 0; i < (int)content.size(); ++i) {
            std::cout << i << " - " << content[i] << " - " << (int)content[i] << std::endl;
        }
        std::cout << "content=" << content << std::endl;
    }
};

A a;

int main(int argc, char *argv[]) {
    sylar::EnvMgr::GetInstance()->addHelper("s", "start with the terminal");
    sylar::EnvMgr::GetInstance()->addHelper("d", "run as daemon");
    sylar::EnvMgr::GetInstance()->addHelper("p", "print help");

    if (!sylar::EnvMgr::GetInstance()->init(argc, argv)) {
        sylar::EnvMgr::GetInstance()->printHelper();
        return 0;
    }
    std::cout << "exe=" << sylar::EnvMgr::GetInstance()->getExe() << std::endl;
    std::cout << "cwd=" << sylar::EnvMgr::GetInstance()->getCwd() << std::endl;

    std::cout << "path=" << sylar::EnvMgr::GetInstance()->getEnv("PATH", "xxx") << std::endl;
    std::cout << "test=" << sylar::EnvMgr::GetInstance()->getEnv("TEST", "") << std::endl;
    
    std::cout << "setEnv() : " << sylar::EnvMgr::GetInstance()->setEnv("TEST_YY", "yy") << std::endl;
    std::cout << "test_yy=" << sylar::EnvMgr::GetInstance()->getEnv("TEST_YY", "") << std::endl;

    if (sylar::EnvMgr::GetInstance()->has("p")) {
        sylar::EnvMgr::GetInstance()->printHelper();
    }
    
    return 0;
}
