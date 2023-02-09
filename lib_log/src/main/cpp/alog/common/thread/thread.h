//
// Created by 张德文 on 2022/4/22.
//

#ifndef ANDROIDAVLEARN_THREAD_H
#define ANDROIDAVLEARN_THREAD_H

#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>

#include "condition.h"
#include "runnable.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "thread"
#include "LogUtil.h"

class ThreadUtil {
public:
    static void yield() {
        ::sched_yield();
    }

    static void sleep(unsigned int _sec) {
        ::sleep(_sec);
    }

    static void usleep(unsigned int _usec) {
        ::usleep(_usec);
    }

    static pthread_t currentthreadid() {
        return pthread_self();
    }

    static bool isruning(pthread_t _id) {
        int ret = pthread_kill(_id, 0);

        switch (ret) {
            case 0: return true;
            case ESRCH: return false;
            case EINVAL: return false;
        }

        return false;
    }

};

class Thread {
private:

    class RunnableReference {
    public:
        RunnableReference(Runnable* _target)
        : m_target(_target), m_count(0), m_tid(0), m_is_joined(false), m_is_ended(true),
          m_after_time(LONG_MAX), m_periodic_time(LONG_MAX), m_kill_sig(0), m_is_cancel_delay_start(false),
          m_condition(), m_splock(), m_is_in_thread(false) {
            memset(thread_name, 0, sizeof(thread_name));
        }

        ~RunnableReference() {
            delete m_target;
        }

        void AddRef() {
            m_count++;
        }

        void RemoveRef(ScopedSpinLock& _lock) {
            bool  willdel = false;
            m_count--;

            if (0 == m_count) {
                willdel = true;
            }

            _lock.unlock();

            if (willdel) {
                delete this;
            }

        }

    private:
        RunnableReference(const RunnableReference&);
        RunnableReference& operator=(const RunnableReference&);

    public:
        Runnable* m_target;
        int m_count;
        pthread_t m_tid;
        bool m_is_joined;
        bool m_is_ended;
        long m_after_time;
        long m_periodic_time;
        bool m_is_cancel_delay_start;
        Condition m_condition;
        SpinLock m_splock;
        bool m_is_in_thread;
        int m_kill_sig;
        char thread_name[128];
    };

public:
    template<class T>
    explicit Thread(const T& op, const char* _thread_name = nullptr, bool _outside_join = false)
    : m_runnable_ref(nullptr), m_outside_join(_outside_join) {
        m_runnable_ref = new RunnableReference(transform(op));
        ScopedSpinLock lock(m_runnable_ref->m_splock);
        m_runnable_ref->AddRef();

        int ret = pthread_attr_init(&m_attr);

        if (_thread_name) {
            strncpy(m_runnable_ref->thread_name, _thread_name, sizeof(m_runnable_ref->thread_name));
        }

    }

    Thread(const char* _thread_name = nullptr, bool _outside_join = false) {
        m_runnable_ref = new RunnableReference(nullptr);
        ScopedSpinLock lock(m_runnable_ref->m_splock);
        m_runnable_ref->AddRef();

        int ret = pthread_attr_init(&m_attr);

        if (_thread_name) {
            strncpy(m_runnable_ref->thread_name, _thread_name, sizeof(m_runnable_ref->thread_name));
        }
    }

    virtual ~Thread() {
        int ret = pthread_attr_destroy(&m_attr);

        ScopedSpinLock lock(m_runnable_ref->m_splock);
        if (0 != m_runnable_ref->m_tid && !m_runnable_ref->m_is_joined) {
            pthread_detach(m_runnable_ref->m_tid);
        }
        m_runnable_ref->RemoveRef(lock);
    }

    int start(bool* _newone = nullptr) {
        if (_newone) *_newone = false;

        if (isruning()) return 0;

        m_runnable_ref->m_is_ended = false;
        m_runnable_ref->m_is_joined = m_outside_join;
        m_runnable_ref->AddRef();

        int ret = pthread_create(reinterpret_cast<pthread_t *>(&m_runnable_ref->m_tid), &m_attr, start_routine_after, m_runnable_ref);

        if (_newone) *_newone = true;

        return ret;
    }

    int start_after(long after) {
        ScopedSpinLock  lock(m_runnable_ref->m_splock);
        if (isruning()) {
            return 0;
        }
        if (0 != m_runnable_ref->m_tid && !m_runnable_ref->m_is_joined) {
            pthread_detach(m_runnable_ref->m_tid);
        }

        m_runnable_ref->m_condition.cancelAnywayNotify();
        m_runnable_ref->m_is_joined = m_outside_join;
        m_runnable_ref->m_is_ended = false;
        m_runnable_ref->m_after_time = after;
        m_runnable_ref->m_is_cancel_delay_start = false;
        m_runnable_ref->AddRef();

        // 创建线程
        int ret = pthread_create(reinterpret_cast<pthread_t *>(&m_runnable_ref->m_tid), &m_attr, start_routine_after, m_runnable_ref);
//        LOGD("start -> pthread_create:%d", ret);

        if (0 != ret) {
            m_runnable_ref->m_is_ended = true;
            m_runnable_ref->m_after_time = LONG_MAX;
            m_runnable_ref->RemoveRef(lock);
        }

        return ret;
    }

    void cancel_after() {

        ScopedSpinLock lock(m_runnable_ref->m_splock);

        if (!isruning()) {
            return;
        }

        m_runnable_ref->m_is_cancel_delay_start = true;
        m_runnable_ref->m_condition.notifyAll(true);
    }

    int start_periodic(long after, long periodic) {
        ScopedSpinLock  lock(m_runnable_ref->m_splock);
        if (isruning()) {
            return 0;
        }
        if (0 != m_runnable_ref->m_tid && !m_runnable_ref->m_is_joined) {
            pthread_detach(m_runnable_ref->m_tid);
        }

        m_runnable_ref->m_condition.cancelAnywayNotify();
        m_runnable_ref->m_is_joined = m_outside_join;
        m_runnable_ref->m_is_ended = false;
        m_runnable_ref->m_is_cancel_delay_start = false;
        m_runnable_ref->m_after_time = after;
        m_runnable_ref->m_periodic_time = periodic;
        m_runnable_ref->AddRef();

        int ret = pthread_create(reinterpret_cast<pthread_t *>(&m_runnable_ref->m_tid), &m_attr, start_routine_periodic, m_runnable_ref);

        if (0 != ret) {
            m_runnable_ref->m_is_ended = true;
            m_runnable_ref->m_after_time = LONG_MAX;
            m_runnable_ref->m_periodic_time = LONG_MAX;
            m_runnable_ref->RemoveRef(lock);
        }

        return ret;
    }

    void cancel_periodic() {
        ScopedSpinLock lock(m_runnable_ref->m_splock);

        if (!isruning()) {
            return;
        }

        m_runnable_ref->m_is_cancel_delay_start = true;
        m_runnable_ref->m_condition.notifyAll(true);
    }

    int join() const {
        int ret = 0;
        ScopedSpinLock lock(m_runnable_ref->m_splock);
        if (tid() == ThreadUtil::currentthreadid()) {
            return EDEADLK;
        }

        if (isruning()) {
            m_runnable_ref->m_is_joined = true;
            lock.unlock();
            ret = pthread_join(tid(), 0);
        }

        return ret;
    }

    int kill(int sig) const {
        ScopedSpinLock lock(m_runnable_ref->m_splock);
        if (!isruning()) {
            return ESRCH;
        }

        if (!m_runnable_ref->m_is_in_thread) {
            m_runnable_ref->m_kill_sig = sig;
            return 0;
        }

        lock.unlock();

        int ret = pthread_kill(tid(), sig);
        return ret;
    }

    int unsafe_exit() const {

        return 0;
    }

    pthread_t tid() const {
        return m_runnable_ref->m_tid;
    }

    bool isruning() const {
        return !m_runnable_ref->m_is_ended;
    }

    void set_stack_size(size_t _stacksize) {
        if (0 == _stacksize) {
            return;
        }
        int ret = pthread_attr_setstacksize(&m_attr, _stacksize);
    }

private:
#ifdef ANDROID
    static void exit_handler(int _sig) {
        pthread_exit(0);
    }
#endif
    static void init(void *arg) {
        volatile RunnableReference* runnableRef = static_cast<RunnableReference *>(arg);
        ScopedSpinLock lock((const_cast<RunnableReference*>(runnableRef))->m_splock);
        runnableRef->m_is_in_thread = true;

        char szthreadname[128] = {0};
        strncpy(szthreadname, (const char*)runnableRef->thread_name, sizeof(szthreadname));
        if (0 < strnlen(szthreadname, sizeof(szthreadname))) {
            pthread_setname_np(runnableRef->m_tid, szthreadname);
        }

        if (!(0 < runnableRef->m_kill_sig && runnableRef->m_kill_sig <= 32)) {
            return;
        }

        lock.unlock();
        pthread_kill(pthread_self(), runnableRef->m_kill_sig);
    }

    static void cleanup(void* arg) {
        volatile RunnableReference* runnableRef = static_cast<RunnableReference *>(arg);
        ScopedSpinLock lock((const_cast<RunnableReference*>(runnableRef))->m_splock);
        runnableRef->m_is_in_thread = false;
        runnableRef->m_kill_sig = 0;
        runnableRef->m_is_ended = true;

        (const_cast<RunnableReference*>(runnableRef))->RemoveRef(lock);
    }

    static void* start_routine(void* arg) {
        init(arg);
        volatile RunnableReference* runnableRef = static_cast<RunnableReference*>(arg);
        pthread_cleanup_push(&cleanup, arg);
        runnableRef->m_target->run();
        pthread_cleanup_pop(1);
        return 0;
    }

    static void* start_routine_after(void *arg) {

        init(arg);

        volatile RunnableReference* runnableRef = static_cast<RunnableReference*>(arg);

        pthread_cleanup_push(&cleanup, arg)

        if (!runnableRef->m_is_cancel_delay_start) {
            // 等待指定的时间
            (const_cast<RunnableReference*>(runnableRef))->m_condition.wait(runnableRef->m_after_time);
            // 如果未被cancel，则真正执行对应的任务
            if (!runnableRef->m_is_cancel_delay_start) {
                runnableRef->m_target->run();
            }
        }

        pthread_cleanup_pop(1)

        return 0;
    }

    static void* start_routine_periodic(void *arg) {

        init(arg);

        volatile RunnableReference* runnableRef = static_cast<RunnableReference*>(arg);
        pthread_cleanup_push(&cleanup, arg);

            if (!runnableRef->m_is_cancel_delay_start) {
                (const_cast<RunnableReference*>(runnableRef))->m_condition.wait(runnableRef->m_after_time);
                while (!runnableRef->m_is_cancel_delay_start) {
                    runnableRef->m_target->run();

                    if (!runnableRef->m_is_cancel_delay_start) {
                        (const_cast<RunnableReference*>(runnableRef))->m_condition.wait(runnableRef->m_periodic_time);
                    }
                }
            }

        pthread_cleanup_pop(1);
        return 0;
    }

private:
    Thread(const Thread&);
    Thread& operator=(const Thread&);

private:
    RunnableReference* m_runnable_ref;
    pthread_attr_t m_attr;
    bool m_outside_join;
};

inline bool operator==(const Thread& lhs, const Thread& rhs) {
    return pthread_equal(lhs.tid(), rhs.tid()) != 0;
}

inline bool operator!=(const Thread& lhs, const Thread& rhs) {
    return pthread_equal(lhs.tid(), rhs.tid()) == 0;
}

#endif //ANDROIDAVLEARN_THREAD_H
