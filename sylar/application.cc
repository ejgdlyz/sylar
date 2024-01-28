#include "application.h"
#include "sylar/config.h"
#include "sylar/env.h"
#include "sylar/log.h"

namespace sylar {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

static sylar::ConfigVar<std::string>::ptr g_server_work_path = 
        sylar::Config::Lookup("server.work_path", std::string("/home/lambda/apps/work/sylar"), "server work path");

static sylar::ConfigVar<std::string>::ptr g_server_pid_file = 
        sylar::Config::Lookup("server.pid_file", std::string("sylar.pid"), "server pid file");

Application* Application::s_instance = nullptr;

Application::Application() {
    s_instance = this;
}

bool Application::init(int argc, char **argv) {
    m_argc = argc;
    m_argv = argv;

    sylar::EnvMgr::GetInstance()->addHelper("s", "start with the terminal");
    sylar::EnvMgr::GetInstance()->addHelper("d", "run as daemon");
    sylar::EnvMgr::GetInstance()->addHelper("c", "default conf path: ./conf");
    sylar::EnvMgr::GetInstance()->addHelper("p", "print help");

    if (!sylar::EnvMgr::GetInstance()->init(argc, argv)) {
        sylar::EnvMgr::GetInstance()->printHelper();
        return false;
    }

    if (sylar::EnvMgr::GetInstance()->has("p")) {
        sylar::EnvMgr::GetInstance()->printHelper();
        return false;
    }

    int run_type = 0;
    if (sylar::EnvMgr::GetInstance()->has("s")) {
        run_type = 1;
    } 
    if (sylar::EnvMgr::GetInstance()->has("d")) {
        run_type = 2;
    }

    if (run_type == 0) {
        sylar::EnvMgr::GetInstance()->printHelper();
        return false;
    }

    std::string pidfile = g_server_work_path->getValue() + "/" + g_server_pid_file->getValue();
    
    if (sylar::FSUtil::IsRunningPidfile(pidfile)) {
        SYLAR_LOG_ERROR(g_logger) << "server is running: " << pidfile;
        return false;
    }

    std::string conf_path = sylar::EnvMgr::GetInstance()->getAbsolutePath(
            sylar::EnvMgr::GetInstance()->get("c", "conf"));
    
    SYLAR_LOG_INFO(g_logger) << "load conf path: " << conf_path;
    sylar::Config::loadFromConfDir(conf_path);

    if (!sylar::FSUtil::Mkdir(g_server_work_path->getValue())) {
        SYLAR_LOG_FATAL(g_logger) << "create work path [" << g_server_work_path->getValue()
                << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }

    std::ofstream ofs(pidfile);
    if (!ofs) {
        SYLAR_LOG_ERROR(g_logger) << "open pidfile: " << pidfile << " failed.";
        return false;
    }

    ofs << getpid();
    return true;
}

bool Application::run() {
    return true;
}

bool Application::getServer(const std::string type, std::vector<TcpServer::ptr>& servers) {
    return false;
}

void Application::listAllServer(std::map<std::string, std::vector<TcpServer::ptr>>& servers) {

}

}