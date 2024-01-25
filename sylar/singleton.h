#ifndef __SYLAR_SINGLE_H__
#define __SYLAR_SINGLE_H__

#include <memory>

namespace sylar {


template <class T, class X = void, int N = 0>
class Singleton {
public:
    /**
     * @description: 单例模式封装
     * @return {返回裸指针}
     */
    static T* GetInstance() {
        static T v;
        return &v;
    }
};

template<class T, class X = void, int N = 0>
class SingletonPtr {
public:
    static std::shared_ptr<T> GetInstance() {
        static std::shared_ptr<T> v(new T);
        return v;
    }
};
}

#endif // __SYLAR_SINGLE_H__