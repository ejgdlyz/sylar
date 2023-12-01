#include <functional>
#include <dlfcn.h>

#include "hook.h"
#include "fiber.h"
#include "iomanager.h"

namespace sylar {

static thread_local bool t_hook_enable = false;  // 线程级别的 hook

#define HOOK_FUN(XX) \
    XX(sleep) \
    XX(usleep)

// 初始化 hook
void hook_init() {
    static bool is_inited = false;
    if (is_inited) {
        return;
    }

#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);  // define a local macro, to get original "sleep" and "usleep"
    HOOK_FUN(XX);   // call global macro HOOK_FUN 
#undef XX
}

struct _HookIniter {
    _HookIniter() {
        hook_init();
    }
};

static _HookIniter s_hook_init;  // 全局静态变量

bool is_hook_enable() {
    return t_hook_enable;
}

void set_hook_enable(bool flag) {
    t_hook_enable = flag;
}

} // namespace sylar

extern "C" {
#define XX(name) name ## _fun name ## _f = nullptr;
    HOOK_FUN(XX);
#undef XX

unsigned int sleep(unsigned int seconds) {
    if (!sylar::t_hook_enable) {
        return sleep_f(seconds);  // 没 hook，返回原函数
    }

    // hook
    sylar::Fiber::ptr fiber = sylar::Fiber::GetThis();
    sylar::IOManager* iom = sylar::IOManager::GetThis();
    
    // 加一个定时器，让它在执行的时间之后 schedule 起来
    iom->addTimer(seconds * 1000, std::bind((void(sylar::Scheduler::*)
            (sylar::Fiber::ptr, int thread))&sylar::IOManager::schedule, iom, fiber, -1), false);
    sylar::Fiber::YieldToHold();

    return 0;
}

int usleep(useconds_t usec) {
    if (!sylar::t_hook_enable) {
        return usleep_f(usec);  // 没 hook，返回原函数
    }

    // hook
    sylar::Fiber::ptr fiber = sylar::Fiber::GetThis();
    sylar::IOManager* iom = sylar::IOManager::GetThis();

    // 加一个定时器，让它在执行的时间之后 schedule 起来
    iom->addTimer(usec / 1000, std::bind((void(sylar::Scheduler::*)
            (sylar::Fiber::ptr, int thread))&sylar::IOManager::schedule, iom, fiber, -1), false);
    sylar::Fiber::YieldToHold();

    return 0;
}

}  // extern "C"