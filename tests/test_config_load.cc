#include "sylar/config.h"
#include "sylar/log.h"
#include "sylar/env.h"
#include <yaml-cpp/yaml.h>
#include <iostream>

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_loadconf() {
    sylar::Config::loadFromConfDir("test_conf");
    std::cout << "=========" << std::endl;
    sleep(10);
    sylar::Config::loadFromConfDir("test_conf");
}

int main(int argc, char *argv[]) {
    sylar::EnvMgr::GetInstance()->init(argc, argv);
    sylar::EnvMgr::GetInstance()->printHelper();
    test_loadconf();
    return 0;
}
