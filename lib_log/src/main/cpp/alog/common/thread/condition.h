//
// Created by 张德文 on 2022/4/22.
//

#ifndef _CONDITION_H
#define _CONDITION_H

#include <errno.h>
#include <pthread.h>
#include <climits>
#include <sys/time.h>

#include "atomic_oper.h"
#include "lock.h"


class Condition {

public:

    Condition(): m_condition(), m_mutex(), m_anyway_notify(0) {
        int ret = pthread_cond_init(&m_condition, 0);
        // 检查一下返回值

    }

    ~Condition() {
        int ret = pthread_cond_destroy(&m_condition);

        // 检查返回值

    }

    void wait(ScopedLock& lock) {
        int ret = 0;

        if (!atomic_cas32(&m_anyway_notify, 0, 1)) {
            ret = pthread_cond_wait(&m_condition, &(lock.internal().internal()));
        }

        m_anyway_notify = 0;

    }

    int wait(ScopedLock& lock, long millisecond) {

        struct timespec ts;
        makeTimeout(&ts, millisecond);

        int ret = 0;

        if (!atomic_cas32(&m_anyway_notify, 0, 1)) {
            ret = pthread_cond_timedwait(&m_condition, &(lock.internal().internal()), &ts);
        }

        m_anyway_notify = 0;

        return ret;
    }

    void wait() {
        ScopedLock scopedLock(m_mutex);
        wait(scopedLock);
    }

    int wait(long millisecond) {
        ScopedLock  scopedLock(m_mutex);
        return wait(scopedLock, millisecond);
    }

    void notifyOne() {
        int ret = pthread_cond_signal(&m_condition);
    }

    void notifyOne(ScopedLock lock) {
        notifyOne();
    }

    void notifyAll(bool anywaynofify = false) {
        if (anywaynofify) {
            m_anyway_notify = 1;
        }

        int ret = pthread_cond_broadcast(&m_condition);

        // 检查返回值

    }

    void notifyAll(ScopedLock lock, bool anywaynotify = false) {
        notifyAll(anywaynotify);
    }

    void cancelAnywayNotify() {
        m_anyway_notify = 0;
    }


private:
    // 禁止拷贝构造
    Condition(const Condition&);
    Condition& operator=(const Condition&);

    static void makeTimeout(struct timespec* pts, long millisecond) {
        struct timeval tv;
        gettimeofday(&tv, 0);

        pts->tv_sec = millisecond / 1000 + tv.tv_sec;
        pts->tv_nsec = (millisecond % 1000) * 1000 * 1000 + tv.tv_usec * 1000;

        pts->tv_sec += pts->tv_nsec / (1000 * 1000 * 1000);
        pts->tv_nsec = pts->tv_nsec % (1000 * 1000 * 1000);
    }

private:
    pthread_cond_t m_condition;
    Mutex m_mutex;
    volatile unsigned int m_anyway_notify;
};

#endif // _CONDITION_H
