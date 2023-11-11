#ifndef __SYLAR_THREAD_H__
#define __SYLAR_THREAD_H__

#include <thread>
#include <functional>
#include <memory>
#include <pthread.h>
#include <string>
#include <semaphore.h>
#include <stdint.h>

namespace sylar {

class Semaphore {
public:
    Semaphore(uint32_t count = 0);
    ~Semaphore();


    void wait();
    void notify();
private:
    Semaphore(const Semaphore&) = delete;
    Semaphore(Semaphore&&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;

private:
    sem_t m_semaphore;

};


template<class T>
class ScopedLockImpl {
public:
    ScopedLockImpl(T& mutex)
            :m_mutex(mutex) {
        m_mutex.lock();
        m_locked = true;
    }

    ~ScopedLockImpl () {
        unlock();
    }

    void lock() {
        if (!m_locked) {
            m_mutex.lock();
            m_locked = true;
        }
    }

    void unlock() {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;
    bool m_locked;
};

template<class T>
class ReadScopedLockImpl {
public:
    ReadScopedLockImpl(T& mutex)
            :m_mutex(mutex) {
        m_mutex.rdlock();
        m_locked = true;
    }

    ~ReadScopedLockImpl () {
        unlock();
    }

    void lock() {
        if (!m_locked) {
            m_mutex.rdlock();
            m_locked = true;
        }
    }

    void unlock() {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;
    bool m_locked;
};

template<class T>
class WriteScopedLockImpl {
public:
    WriteScopedLockImpl(T& mutex)
            :m_mutex(mutex) {
        m_mutex.wrlock();
        m_locked = true;
    }

    ~WriteScopedLockImpl () {
        unlock();
    }

    void lock() {
        if (!m_locked) {
            m_mutex.wrlock();
            m_locked = true;
        }
    }

    void unlock() {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;
    bool m_locked;
};

class RWMutex {
public:
    typedef ReadScopedLockImpl<RWMutex> ReadLock;
    typedef WriteScopedLockImpl<RWMutex> WriteLock;

    RWMutex() {
        pthread_rwlock_init(&m_lock, nullptr);
    }

    ~RWMutex() {
        pthread_rwlock_destroy(&m_lock);
    }

    void rdlock() {
        pthread_rwlock_rdlock(&m_lock);
    }

    void wrlock() {
        pthread_rwlock_wrlock(&m_lock);
    }

    void unlock() {
        pthread_rwlock_unlock(&m_lock);
    }
private:    
    pthread_rwlock_t m_lock;
};

class Thread {
public:
    typedef std::shared_ptr<Thread> ptr;
    Thread(std::function<void()>cb, const std::string& name);
    ~Thread();

    const std::string& getName() const { return m_name;}
    pid_t  getId() const { return m_id;}

    void join();

    static Thread* GetThis();                       // 获得当前线程引用
    static const std::string& GetName();            // for logger
    static void SetName(const std::string& name);   // 通过线程局部变量，主线程也可以命名
private:
    Thread(const Thread&) = delete;    
    Thread(Thread&&) = delete;
    Thread& operator=(const Thread&) = delete;

    static void* run(void* args);

private:    
    pid_t m_id = -1;                      // 线程 id
    pthread_t m_thread = 0;               // 线程结构
    std::function<void()> m_cb;           // 线程函数
    std::string m_name;                   // 线程名

    Semaphore m_semaphore;                // 信号量
};

}


#endif // __SYLAR_THREAD_H__
