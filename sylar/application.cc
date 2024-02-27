#include <functional>
#include <iostream>
#include "application.h"
#include "sylar/daemon.h"
#include "sylar/config.h"
#include "sylar/env.h"
#include "sylar/log.h"
#include "sylar/module.h"
#include "sylar/worker.h"

namespace sylar {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

static sylar::ConfigVar<std::string>::ptr g_server_work_path = 
        sylar::Config::Lookup("server.work_path", std::string("/home/lambda/apps/work/sylar"), "server work path");

static sylar::ConfigVar<std::string>::ptr g_server_pid_file = 
        sylar::Config::Lookup("server.pid_file", std::string("sylar.pid"), "server pid file");

struct HttpServerConf {
    std::vector<std::string> addrs;
    int keepalive = 0;
    int timeout = 1000 * 2 * 60;
    int ssl = 0;
    std::string name = "sylar/0.0";
    std::string cert_file;
    std::string key_file;
    std::string accept_worker;
    std::string process_worker;
    
    bool operator==(const HttpServerConf& rhs) const {
        return addrs == rhs.addrs && keepalive == rhs.keepalive 
                && timeout == rhs.timeout && name == rhs.name && ssl == rhs.ssl
                && cert_file == rhs.cert_file && key_file == rhs.key_file
                && accept_worker == rhs.accept_worker && process_worker == rhs.process_worker;
    }

    bool isValid() const {
        return !addrs.empty();
    }
};

// 偏特化 ： 配置系统与自定义类型的转换
template <>
class LexicalCast<std::string, HttpServerConf> {
public:
    HttpServerConf operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        HttpServerConf conf;
        conf.keepalive = node["keepalive"].as<int>(conf.keepalive);  // 放一个默认值，防止配置文件中不存在 
        conf.timeout = node["timeout"].as<int>(conf.timeout);
        conf.name = node["name"].as<std::string>(conf.name);
        conf.ssl = node["ssl"].as<int>(conf.ssl);
        conf.cert_file = node["cert_file"].as<std::string>(conf.cert_file);
        conf.key_file = node["key_file"].as<std::string>(conf.key_file);
        conf.accept_worker = node["accept_worker"].as<std::string>();
        conf.process_worker = node["process_worker"].as<std::string>();
        if (node["address"].IsDefined()) {
            for (size_t i = 0; i < node["address"].size(); ++i) {
                conf.addrs.push_back(node["address"][i].as<std::string>());
            }
        }
        return conf;
    }
};

// string -> YAML node
template <>
class LexicalCast<HttpServerConf, std::string> {
public:
    std::string operator()(const HttpServerConf& conf) {
        YAML::Node node;
        node["name"] = conf.name;
        node["keepalive"] = conf.keepalive;
        node["timeout"] = conf.timeout;
        node["ssl"] = conf.ssl;
        node["cert_file"] = conf.cert_file;
        node["key_file"] = conf.key_file;
        node["accept_worker"] = conf.accept_worker;
        node["process_worker"] = conf.process_worker;
        for (auto& addr : conf.addrs) {
            node["address"].push_back(addr);
        }

        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

static sylar::ConfigVar<std::vector<HttpServerConf>>::ptr g_http_servers_conf = 
        sylar::Config::Lookup("http_servers", std::vector<HttpServerConf>(), "http servers config");

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

    bool is_print_help = false;
    if (!sylar::EnvMgr::GetInstance()->init(argc, argv)) {
        is_print_help = true;
    }

    if (sylar::EnvMgr::GetInstance()->has("p")) {
        is_print_help = true;
    }

    std::string conf_path = sylar::EnvMgr::GetInstance()->getConfigPath();  // 获取配置路径，以当前可执行文件为相对路径，默认取 conf
    SYLAR_LOG_INFO(g_logger) << "load conf path: " << conf_path;
    sylar::Config::loadFromConfDir(conf_path);                              // 加载配置

    ModuleMgr::GetInstance()->init();
    std::vector<Module::ptr> modules;
    ModuleMgr::GetInstance()->listAll(modules);
    
    for (auto& module : modules) {
        module->onBeforeArgsParse(argc, argv);
    }

    if (is_print_help) {
        sylar::EnvMgr::GetInstance()->printHelper();
        return false;
    }
    
    for (auto& module : modules) {
        module->onAfterArgsParse(argc, argv);
    }
    modules.clear();

    int run_type = 0;
    if (sylar::EnvMgr::GetInstance()->has("s")) {  // 命令行方式启动
        run_type = 1;
    } 
    if (sylar::EnvMgr::GetInstance()->has("d")) {  // 守护进程方式启动
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

    // 创建进程的 工作路径，可以存放系统的信息
    if (!sylar::FSUtil::Mkdir(g_server_work_path->getValue())) {
        SYLAR_LOG_FATAL(g_logger) << "create work path [" << g_server_work_path->getValue()
                << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }

    return true;
}

bool Application::run() {
    bool is_daemon = sylar::EnvMgr::GetInstance()->has("d");
    return start_daemon(m_argc, m_argv
            , std::bind(&Application::main, this, std::placeholders::_1, std::placeholders::_2)
            , is_daemon);
}

int Application::main(int argc, char** argv) {
    std::string pidfile = g_server_work_path->getValue() + "/" + g_server_pid_file->getValue();
    
    // 写入进程 id (子进程id if daemon)
    std::ofstream ofs(pidfile);
    if (!ofs) {
        SYLAR_LOG_ERROR(g_logger) << "open pidfile: " << pidfile << " failed.";
        return false;
    }
    ofs << getpid();

    m_mainIOManager.reset(new sylar::IOManager(1, true, "main"));
    m_mainIOManager->schedule(std::bind(&Application::run_fiber, this));
    m_mainIOManager->addTimer(1000, [](){}, true);
    m_mainIOManager->addTimer(2000, [](){
        // SYLAR_LOG_INFO(g_logger) << "hello";
    }, true);
    m_mainIOManager->stop();
    return 0;
}

int Application::run_fiber() {
    sylar::WorkerMgr::GetInstance()->init();
    // server 放在协程里初始化
    auto http_confs = g_http_servers_conf->getValue();
    for (auto& conf : http_confs) {
        SYLAR_LOG_INFO(g_logger) << std::endl << LexicalCast<HttpServerConf, std::string>()(conf);  // HttpServerConf -> string
        
        std::vector<Address::ptr> p_addrs;
        for(auto& addr : conf.addrs) {
            size_t pos = addr.find(":");
            if (pos == std::string::npos) {
                // SYLAR_LOG_ERROR(g_logger) << "invalid address: " << addr;
                p_addrs.push_back(UnixAddress::ptr(new UnixAddress(addr)));
                continue;
            }
            int32_t port = atoi(addr.substr(pos + 1).c_str());
            // 解析地址, 127.0.0.1
            auto p_addr = sylar::IPAddress::Create(addr.substr(0, pos).c_str(), port);
            if (p_addr) {
                p_addrs.push_back(p_addr);
                continue;
            } 
            // 地址解析不成功，解析网卡 
            std::vector<std::pair<Address::ptr, uint32_t>> results;
            if (sylar::Address::GetInterfaceAddresses(results, addr.substr(0, pos))) {
                for (auto& res : results) {
                    auto ip_addr = std::dynamic_pointer_cast<IPAddress>(res.first);
                    if (ip_addr) {
                        ip_addr->setPort(atoi(addr.substr(pos + 1).c_str()));
                    }
                    p_addrs.push_back(ip_addr);
                }
                continue;
            }
            auto aaddr = sylar::Address::LookupAny(addr);
            if (aaddr) {
                p_addrs.push_back(aaddr);
                continue;
            }
            SYLAR_LOG_ERROR(g_logger) << "invalid address: " << addr;
            _exit(0);
        }

        IOManager* accept_worker = sylar::IOManager::GetThis();
        IOManager* process_worker = sylar::IOManager::GetThis();
        if (!conf.accept_worker.empty()) {
            accept_worker = sylar::WorkerMgr::GetInstance()->getAsIOManager(conf.accept_worker).get();
            if (!accept_worker) {
                SYLAR_LOG_ERROR(g_logger) << "accept_worker: " << conf.accept_worker
                        << " not exists.";
                _exit(0);
            }
        }
        if (!conf.process_worker.empty()) {
            process_worker = sylar::WorkerMgr::GetInstance()->getAsIOManager(conf.process_worker).get();
            if (!process_worker) {
                SYLAR_LOG_ERROR(g_logger) << "process_worker: " << conf.process_worker
                        << " not exists.";
                _exit(0);
            }
        }
        // 创建 httpserver
        sylar::http::HttpServer::ptr server(new sylar::http::HttpServer(conf.keepalive
                , process_worker, accept_worker));
        std::vector<Address::ptr> failed_addrs;
        if (!server->bind(p_addrs, failed_addrs, conf.ssl)) {
            for (auto& f_addr : failed_addrs) {
                SYLAR_LOG_ERROR(g_logger) << "failed to bind address: " << *f_addr;
            }
            _exit(0); // 绑定失败，直接退出
        }
        
        if (conf.ssl) {
            if (!server->loadCertificates(conf.cert_file, conf.key_file)) {
                SYLAR_LOG_ERROR(g_logger) << "loadCertificates failure, cert_file" << conf.cert_file
                        << ", key_file=" << conf.key_file;
            }
        }
        
        if (!conf.name.empty()) {
            server->setName(conf.name);
        }
        server->start();
        m_httpServers.push_back(server);
    }

    return 0;
}

bool Application::getServer(const std::string type, std::vector<TcpServer::ptr>& servers) {
    return false;
}

void Application::listAllServer(std::map<std::string, std::vector<TcpServer::ptr>>& servers) {

}

}