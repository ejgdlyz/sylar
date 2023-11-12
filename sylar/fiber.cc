#include <atomic>
#include "fiber.h"
#include "config.h"
#include "macro.h"
#include "log.h"

namespace sylar {

static Logger::ptr g_logger = SYLAR_LOG_NAME("system");

static std::atomic<uint64_t> s_fiber_id {0};
static std::atomic<uint64_t> s_fiber_count {0};

static thread_local Fiber* t_fiber = nullptr;       // 当前线程
static thread_local Fiber::ptr t_threadFiber = nullptr;  // main 协程

static ConfigVar<uint32_t>::ptr g_fiber_stack_size =
        Config::Lookup("fiber.stack_size", (uint32_t)1024 * 1024, "Fiber stack size");      // 协程栈大小， 默认 1M

class MallocStackAllocate {
public:
    static void* Alloc(size_t size) {
        return malloc(size);
    }    

    static void Dealloc(void* vp, size_t size) {
        return free(vp);
    }
};

using StackAlloc = MallocStackAllocate;

Fiber::Fiber() {  // 当前线程的上下文赋给 main 协程，
    m_state = EXEC;
    SetThis(this); 

    if (getcontext(&m_ctx)) {
        SYLAR_ASSERT2(false, "Fiber::Fiber(): getcontext");
    }

    ++s_fiber_count;

    SYLAR_LOG_DEBUG(g_logger) << "Fiber::Fiber";
}

Fiber::Fiber(std::function<void()> cb, size_t stackSize) 
        :m_id(++s_fiber_id)
        ,m_cb(cb) {
    ++s_fiber_count;
    m_statckSize = stackSize ? stackSize : g_fiber_stack_size->getValue();

    // 协程上下文环境初始化
    m_stack = StackAlloc::Alloc(m_statckSize);
    if (getcontext(&m_ctx)) {
        SYLAR_ASSERT2(false, "Fiber::Fiber(cb): getcontext");
    }
    m_ctx.uc_link = nullptr;                // 当前协程结束后的返回点
    m_ctx.uc_stack.ss_sp = m_stack;         // 当前协程的栈空间起始地址
    m_ctx.uc_stack.ss_size = m_statckSize;  // 当前协程的栈空间大小

    makecontext(&m_ctx, &Fiber::MainFunc, 0);   // 入口函数与协程上下文环境绑定

    SYLAR_LOG_DEBUG(g_logger) << "Fiber::Fiber, id = " << m_id;

}

Fiber::~Fiber() {
    --s_fiber_count;
    if (m_stack) {
        SYLAR_ASSERT(m_state == TREM || m_state == EXCEPT || m_state == INIT );
        
        // 回收栈
        StackAlloc::Dealloc(m_stack, m_statckSize);
    } else {                            // 主协程
        SYLAR_ASSERT(!m_cb);            // main_fiber's cb is null
        SYLAR_ASSERT(m_state == EXEC);  // main_fiber's state is exec
        
        Fiber* cur = t_fiber;
        if (cur == this) {
            SetThis(nullptr);  
        }
    }

    SYLAR_LOG_DEBUG(g_logger) << "Fiber::~Fiber, id = " << m_id;

}

void Fiber::reset(std::function<void()> cb) {
    SYLAR_ASSERT(m_stack);
    SYLAR_ASSERT(m_state == TREM || m_state == EXCEPT || m_state == INIT);

    m_cb = cb;
    if (getcontext(&m_ctx)) {
        SYLAR_ASSERT2(false, "Fiber::reset: getcontext");
    }
    m_ctx.uc_link = nullptr;                
    m_ctx.uc_stack.ss_sp = m_stack;         
    m_ctx.uc_stack.ss_size = m_statckSize;  

    makecontext(&m_ctx, &Fiber::MainFunc, 0);   

    m_state = INIT;
}

void Fiber::swapIn() {
    // 目标协程唤醒到当前执行状态
    SetThis(this);
    SYLAR_ASSERT(m_state != EXEC);
    m_state = EXEC;

    if (swapcontext(&t_threadFiber->m_ctx, &m_ctx)) {
        SYLAR_ASSERT2(false, "Fiber::swapIn: swapcontext");
    }
}

void Fiber::swapOut() {
    // 当前协程 Yield 到后台，唤醒 main 协程
    SetThis(t_threadFiber.get());
    
    if (swapcontext(&m_ctx, &t_threadFiber->m_ctx)) {
        SYLAR_ASSERT2(false, "Fiber::swapOut: swapcontext");
    }


}

void Fiber::SetThis(Fiber* fiber) {
    t_fiber = fiber;
}

Fiber::ptr Fiber::GetThis() {
    if (t_fiber) {
        return t_fiber->shared_from_this();
    }

    Fiber::ptr main_fiber(new Fiber);
    SYLAR_ASSERT(t_fiber == main_fiber.get());

    t_threadFiber = main_fiber;
    
    return t_fiber->shared_from_this();
}

// 当前协程切换到后台并设置为 Ready 状态
void Fiber::YieldToReady() {
    Fiber::ptr cur = GetThis();
    SYLAR_ASSERT(cur->m_state == EXEC);
    cur->m_state = READY;
    cur->swapOut();
}

// 当前协程切换到后台并设置为 Hold 状态
void Fiber::YieldToHold() {
    Fiber::ptr cur = GetThis();
    SYLAR_ASSERT(cur->m_state == EXEC);
    cur->m_state = HOLD;
    cur->swapOut();
}

uint64_t Fiber::TotalFibers() {
    return s_fiber_count;
}

void Fiber::MainFunc() {
    Fiber::ptr cur = GetThis();
    SYLAR_ASSERT(cur);
    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TREM;
    }  catch (std::exception& e) {
        cur->m_state = EXCEPT;
        SYLAR_LOG_ERROR(g_logger) << "Fiber Except: " << e.what();
    } catch (...) {
        SYLAR_LOG_ERROR(g_logger) << "Fiber Except.";;
    }
}

uint64_t Fiber::GetFiberId() {
    if (t_fiber) {
        return t_fiber->getId();
    }
    return 0;
}

}