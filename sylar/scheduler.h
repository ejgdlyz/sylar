#ifndef __SYLAT_SCHEDULER_H__
#define __SYLAT_SCHEDULER_H__

#include <vector>
#include <memory>
#include <functional>
#include <atomic>
#include <list>
#include "fiber.h"
#include "thread.h"

namespace sylar {

class Scheduler {
public:
    typedef std::shared_ptr<Scheduler> ptr;
    typedef Mutex MutexType;

    Scheduler(size_t thread_size = 1, bool use_caller = true, const std::string& name = "");
    virtual ~Scheduler();

    const std::string& getName() const { return m_name;}

    // 当前协程调度器
    static Scheduler* GetThis();       
    // 协程调度器中的主协程
    static Fiber* GetMainFiber();

    void start();
    void stop();

    template <class FiberOrCb>
    void schedule(FiberOrCb fc, int thread = -1) {
        bool need_tickle = m_fibers.empty();
        {
            MutexType::Lock lock(m_mutex);
            need_tickle = scheduleNoLock(fc, thread); 
        }

        if (need_tickle) {
            tickle();
        }
    }

    // 批量放入
    template <class InputIterator>
    void schedule(InputIterator begin, InputIterator end) {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            while (begin != end) {
                need_tickle = scheduleNoLock(&*begin) || need_tickle;  // 这里传入的是指针，会进行 swap
                ++begin;
            }
        }

        if (need_tickle) {
            tickle();
        }

    }

protected:
    virtual void tickle();
    void run();
    virtual bool stopping();
    virtual void idle();

    void setThis();
private:
    template <class FiberOrCb>
    bool scheduleNoLock(FiberOrCb fc, int thread) {
        bool need_tickle = m_fibers.empty();
        FiberAndThread ft(fc, thread);
        if (ft.fiber || ft.cb) {
            m_fibers.push_back(ft);  // 放入协程等待队列
        }
        return need_tickle;         // 当放入一个fc，就需要通知线程有任务来了
    } 
private:
    struct FiberAndThread {
        Fiber::ptr fiber;           // 协程
        std::function<void()> cb;   // 回调
        int thread;                 // 线程 id， 指定协程调度器在哪一个线程上执行

        FiberAndThread(Fiber::ptr f, int thr)
                :fiber(f), thread(thr) {}
        
        FiberAndThread(Fiber::ptr* f, int thr)
                :thread(thr) {
            fiber.swap(*f);
        }

        FiberAndThread(std::function<void()> c, int thr)
                :cb(c), thread(thr) {
            
        }

        FiberAndThread(std::function<void()>* c, int thr)
                :thread(thr) {
            cb.swap(*c);
        }

        FiberAndThread(): thread(-1) {

        }
        
        void reset() {
            fiber = nullptr;
            cb = nullptr;
            thread = -1;
        }
    };
private:
    MutexType m_mutex;
    std::vector<Thread::ptr> m_threads;
    std::list<FiberAndThread> m_fibers;         // 即将执行的协程
    // std::map<int, std::list<FiberAndThread>> m_thrFibers; 
    Fiber::ptr m_rootFiber;                 // 主协程
    std::string m_name;

protected:
    // 线程状态
    std::vector<int> m_threadIds;       // 线程 id
    size_t m_threadCount = 0;           // 总线程数
    std::atomic<size_t> m_activeThreadCount = {0};     // 活跃线程数
    std::atomic<size_t> m_idleThreadCount = {0};       // 空闲线程数
    bool m_stopping = true;             // 默认不启动       
    bool m_autoStop = false;            // 
    int m_rootThread = 0;               // 主线程 id et. use_caller's id
};

}

#endif  // __SYLAT_SCHEDULER_H__