#include "scheduler.h"
#include "log.h"
#include "macro.h"

namespace sylar {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

static thread_local Scheduler* t_scheduler = nullptr;       // 线程中获取调度器指针
static thread_local Fiber* t_fiber = nullptr;               // 此协程的主协程函数

Scheduler::Scheduler(size_t thread_size, bool use_caller, const std::string& name) 
        :m_name(name) {
    SYLAR_ASSERT(thread_size > 0);
    if (use_caller) {
        sylar::Fiber::GetThis();      // 初始化一个主协程
        --thread_size;                // 主协程占用一个线程

        SYLAR_ASSERT(GetThis() == nullptr);  // 当前线程必须没有调度器存在
        t_scheduler = this;
        // 当线程是一个新线程时，新线程的主线程并不会参与协程调度
        // 所以，创建一个新的线程专门去做Schedule 的流程方法
        // run() 方法中去执行
        // 使用了 use_caller 意味着 main 协程是不能参与到 run() 方法中的
        // 所以，通过此方法进行调度
        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
        sylar::Thread::SetName(m_name);

        // 线程中声明一个调度器，在把当前线程放入调度器中时，
        // 它的主协程不再是线程的主协程，而是执行 run 的主协程
        t_fiber = m_rootFiber.get();

        m_rootThread = sylar::GetThreadId();  // 主线程 id
        m_threadIds.push_back(m_rootThread);
        
    } else {
        m_rootThread = -1;
    }
    
    m_threadCount = thread_size;
    
}

Scheduler::~Scheduler() {
    SYLAR_ASSERT(m_stopping);

    if (GetThis() == this) {
        t_scheduler = nullptr;
    }

}

// 当前协程调度器
Scheduler* Scheduler::GetThis() {
    return t_scheduler;
}

// 协程调度器中的主协程
Fiber* Scheduler::GetMainFiber() {
    return t_fiber;
}

void Scheduler::start() {
    // 启动线程
    MutexType::Lock lock(m_mutex);
    if (!m_stopping) {
        return;
    }

    m_stopping = false;  // 启动
    SYLAR_ASSERT(m_threads.empty());

    m_threads.resize(m_threadCount);
    for (size_t i = 0; i < m_threadCount; ++i) {
        m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this)
                , m_name + "_" + std::to_string(i)));
        
        // 因为线程的构造函数放了一个信号量，所以，线程开始的时候，其id一定被初始化好了
        m_threadIds.push_back(m_threads[i]->getId());
    }

    lock.unlock();

    // if (m_rootFiber) {
    //     m_rootFiber->call();
    //     // m_rootFiber->swapIn();
    //     SYLAR_LOG_INFO(g_logger) << "call out, state = " 
    //         << m_rootFiber->getState();
    // }
}

void Scheduler::stop() {
    // stop 两种情况： 用 use_caller 与 不用 use_caller
    // 1. 使用 use_caller 的一定要在创建的线程中执行 stop
    // 2. 没有使用，则可以在任意非自己的所在的线程执行 stop

    m_autoStop = true;
    // 1. 只有一个线程情况
    // m_rootFiber 是执行 scheduler::run() 的协程
    if (m_rootFiber && m_threadCount == 0  
            && (m_rootFiber->getState() == Fiber::TREM || m_rootFiber->getState() == Fiber::INIT)) {
        
        SYLAR_LOG_INFO(g_logger) << this << " stopped";
        m_stopping = true;

        if (stopping()) {  // 子类实现
            return;
        }
    }
    
    // 2. 每个线程 tickle() 以及 主线程 tickle() 后，返回
    // 如果想使用创建scheduler的线程，stop 一定要在创建的线程中执行。
    // bool exit_on_this_fiber = false;
    if (m_rootThread != -1) {  // 说明是 use_caller
        SYLAR_ASSERT(GetThis() == this);

    } else {
        SYLAR_ASSERT(GetThis() != this);
    }

    m_stopping = true;
    // 停止了，唤醒所有线程，让他们结束
    for(size_t i = 0; i < m_threadCount; ++i) {
        tickle();   // 类似于信号量，唤醒线程
    }

    if (m_rootFiber) {
        tickle();
    }

    if (m_rootFiber) {
        while (!stopping()) {
            if (m_rootFiber->getState() == Fiber::TREM 
                    || m_rootFiber->getState() == Fiber::EXCEPT) {
                
                // if m_rootFiber 停止，就新建一个方法去 run
                m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
               SYLAR_LOG_INFO(g_logger) << "root fiber is term, reset";

            }
            m_rootFiber->call();
        }
    }
    
    std::vector<Thread::ptr> tmp_threads;
    {
        MutexType::Lock lock(m_mutex);
    }
    
    for(auto& thread : tmp_threads) {
        thread->join();
    }

    // if (exit_on_this_fiber) {

    // }
}

void Scheduler::setThis() {
    t_scheduler = this;
}

// 真正执行协程调度的方法
// 将当前线程的 Scheduler 放进来
void Scheduler::run() {
    setThis();

    if (sylar::GetThreadId() != m_rootThread) {  // 即 run() 所在 id
        t_fiber = Fiber::GetThis().get();
    } 

    // 没有任务做的时候，应该执行 idle 协程 
    // idle() 具体做什么，取决于子类
    // 因为协程调度器中没有任务，又不能使线程终止
    // 1. idle 可以在没有任务的情况下，一直占用CPU
    // 2. 或者 没过一段时间 sleep，让出执行时间
    // 具体的实现有子类完成
    // 例子：基于 epoll 的异步 socket I/O
    // 如果想使 调度器支持 epoll，那么 idle 陷入在 epoll_wait 中
    // 通过 epoll_wait 来唤醒它，执行任务
    Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
    Fiber::ptr cb_fiber;  // 回调函数， function 函数的协程

    FiberAndThread ft;
    while (true) {
        ft.reset();
        bool tickle_me = false;

        // 协程消息队列中取协程
        {
            MutexType::Lock lock(m_mutex);
            auto it = m_fibers.begin();
            while (it != m_fibers.end()) {
                
                // 如果一个任务已经指定好了在哪个线程执行，
                // 当前正在执行 run 的线程 不是它所指定的，跳过
                // 通知其他线程去处理
                if (it->thread != -1 && it->thread != sylar::GetThreadId()) {
                    ++it;
                    tickle_me = true;  // 通知其他线程
                    continue;
                }

                SYLAR_ASSERT(it->fiber || it->cb);

                if (it->fiber && it->fiber->getState() == Fiber::EXEC) {
                    ++it;
                    continue;
                }

                ft = *it;
                m_fibers.erase(it);
                // m_fibers.erase(it++);
                // ++m_activeThreadCount;
                break;

            }
            tickle_me |= it != m_fibers.end();
        }

        if (tickle_me) {
            tickle();
        }

        if (ft.fiber && (ft.fiber->getState() != Fiber::TREM
                && ft.fiber->getState() != Fiber::EXCEPT)) {
            ++m_activeThreadCount;
            ft.fiber->swapIn();  // 唤醒并执行
            --m_activeThreadCount;

            if (ft.fiber->getState() == Fiber::READY) {
                schedule(ft.fiber);  // 继续执行
            } else if (ft.fiber->getState() != Fiber::TREM 
                    && ft.fiber->getState() != Fiber::EXCEPT) {
                ft.fiber->m_state = Fiber::HOLD;  // 让出执行时间，状态变为 hold
            }
            ft.reset();
        } else if (ft.cb) {
            if (cb_fiber) {
                cb_fiber->reset(ft.cb);  // 使用的 ->，所以访问的是 Fiber::reset
            } else {
                cb_fiber.reset(new Fiber(ft.cb)); // 智能指针的 reset
                // ft.cb = nullptr;
            }
            ft.reset();
            ++m_activeThreadCount;
            cb_fiber->swapIn();  // 新创建的协程 swapIn()
            --m_activeThreadCount;
            
            if (cb_fiber->getState() == Fiber::READY) {
                schedule(cb_fiber);
                cb_fiber.reset();
            } else if (cb_fiber->getState() == Fiber::EXCEPT
                    || cb_fiber->getState() == Fiber::TREM) {
                // 执行已经完成
                cb_fiber->reset(nullptr);  // 不是智能指针 reset，不会析构
            } else {
                cb_fiber->m_state = Fiber::HOLD;
                cb_fiber.reset();
            }
        } else {  // 没有任务执行，就执行 idle
            if (idle_fiber->getState() == Fiber::TREM) {
                SYLAR_LOG_INFO(g_logger) << "idle fiber term";
                break;
            }
            ++m_idleThreadCount;
            idle_fiber->swapIn();
            --m_idleThreadCount;

            // idle_fiber 回来
            if (idle_fiber->getState() != Fiber::TREM 
                    && idle_fiber->getState() != Fiber::EXCEPT) {
                idle_fiber->m_state = Fiber::HOLD;
            }
        }
    }
}

void Scheduler::tickle() {
    SYLAR_LOG_INFO(g_logger) << "tickle()";
}

// 任务是否已经执行完成
bool Scheduler::stopping() {
    MutexType::Lock lock(m_mutex);
    return m_autoStop && m_stopping 
        && m_fibers.empty() && m_activeThreadCount == 0;
}

void Scheduler::idle() {
    SYLAR_LOG_INFO(g_logger) << "idle()";
}


}