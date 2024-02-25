#include "worker.h"
#include "config.h"
#include "util.h"

namespace sylar {

static sylar::ConfigVar<std::map<std::string, std::map<std::string, std::string>>>::ptr g_worker_config = 
        sylar::Config::Lookup("workers", std::map<std::string, std::map<std::string, std::string>>(), "worker config");

WorkerGroup::WorkerGroup(uint32_t batch_size, sylar::Scheduler* s)
    : m_batchSize(batch_size)
    , m_finish(false)
    , m_scheduler(s) {
}

WorkerGroup::~WorkerGroup() {
    waitAll();
}

void WorkerGroup::schedule(std::function<void()> cb, int thread) {
    m_fiberSem.wait();
    m_scheduler->schedule(std::bind(&WorkerGroup::doWork, shared_from_this(), cb), thread);
}

void WorkerGroup::doWork(std::function<void()> cb) {
    cb();
    m_fiberSem.notify();
}

void WorkerGroup::waitAll() {
    if (!m_finish) {
        m_finish = true;
        for (uint32_t i = 0; i < m_batchSize; ++i) {
            m_fiberSem.wait();
        }
    }
}

WorkManager::WorkManager()
    : m_stop(false) {
}

void WorkManager::add(Scheduler::ptr s) {
    m_data[s->getName()].push_back(s);
}

Scheduler::ptr WorkManager::get(const std::string& name) {
    auto it = m_data.find(name);
    if (it == m_data.end()) {
        return nullptr;
    }
    if (it->second.size() == 1) {
        return it->second[0];
    }
    return it->second[rand() % it->second.size()];
}

IOManager::ptr WorkManager::getAsIOManager(const std::string& name) {
    return std::dynamic_pointer_cast<IOManager>(get(name));
}


bool WorkManager::init() {
    auto workers = g_worker_config->getValue();
    for (auto& worker: workers) {
        std::string name = worker.first;
        int32_t thread_num = sylar::GetParamValue(worker.second, "thread_num", 1);
        int32_t worker_num = sylar::GetParamValue(worker.second, "worker_num", 1);
        
        for (int32_t i = 0; i < worker_num; ++i) {
            Scheduler::ptr s;
            if (!i) {
                s = std::make_shared<IOManager>(thread_num, false, name);
            } else {
                s = std::make_shared<IOManager>(thread_num, false, name + "-" + std::to_string(i));
            }
            add(s);
        }
    }
    m_stop = m_data.empty();
    return true;
}

void WorkManager::stop() {
    if (m_stop) {
        return;
    }
    for (auto& data : m_data) {
        for (auto& scheduler : data.second) {
            scheduler->schedule([](){});
            scheduler->stop();
        }
    }
    m_data.clear();
    m_stop = true;
}

uint32_t WorkManager::getCount() {
    return m_data.size();
}

std::ostream& WorkManager::dump(std::ostream& os) {
    for (auto& data: m_data) {
        for (auto& scheduler : data.second) {
            scheduler->dump(os) << std::endl;
        }
    }
    return os;
}


}