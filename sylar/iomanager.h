#ifndef __SYLAR_IOMANAGER_H__
#define __SYLAR_IOMANAGER_H__

#include "sylar.h"
#include "scheduler.h"
#include "timer.h"
#include "noncopyable.h"

namespace sylar {

class IOManager : public Scheduler, public TimerManager, public NonCopyable {
public:
    typedef std::shared_ptr<IOManager> ptr;
    typedef RWMutex RWMutexType;

    enum Event {
        NONE    = 0x0,      
        READ    = 0x1,      // EPOLLIN
        WRITE   = 0x4       // EPOLLOUT
    };


private:    
    struct FdContext {
        typedef Mutex MutexType;
        struct EventContext {

            Scheduler* scheduler = nullptr;  // 事件执行的 scheduler
            Fiber::ptr fiber;                // 事件的协程
            std::function<void()> cb;        // 事件的回调函数
        };

        EventContext& getContext(Event event);
        void resetContext(EventContext& evt_ctx);
        void triggerEvent(Event event);
        

        EventContext read;      // 读事件
        EventContext write;     // 写事件
        int fd = 0;             // 事件关联的句柄
        Event events = NONE;    // 已注册的事件
        MutexType mutex;        
    };
public:
    IOManager(size_t thread_size = 1, bool use_caller = true, const std::string& name = "");
    ~IOManager();

    // 1:success, 0:retry, -1:error
    int addEvent(int fd, Event event, std::function<void()> cb = nullptr);     // 
    bool delEvent(int fd, Event event);         // 删除事件
    bool cancelEvent(int fd, Event event);      // 取消事件

    bool cancelAll(int fd);                     // 取消 fd 上所有事件

    static IOManager* GetThis();                // 获取当前的 IOManager

protected:
    void tickle() override;
    bool stopping() override;
    void idle() override;
    void onTimerInsertedAtFront() override;

    void contextResize(size_t size);
    bool stopping(uint64_t& timeout);
private:
    int m_epollfd = 0;
    int m_tickleFds[2];                             // [0]-read, [1]-write

    std::atomic<size_t> m_pendingEventCount = {0};  // 等待执行的事件数量
    RWMutexType m_mutex;
    std::vector<FdContext*>  m_fdContexts ;         // 每个句柄上下文数组
};

}

#endif  // __SYLAR_IOMANAGER_H__