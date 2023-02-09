//
// Created by 张德文 on 2022/4/22.
//

#ifndef _LOCK_H
#define _LOCK_H

#include <unistd.h>
#include "mutex.h"
#include "spinlock.h"

template <typename MutexType>
class BaseScopedLock {

public:

    explicit BaseScopedLock(MutexType& mutex, bool initiallyLocked = true):
        m_mutex(mutex), m_is_locked(false) {
        if (!initiallyLocked) {
            return;
        }
        lock();
    }

    explicit BaseScopedLock(MutexType& mutex, long _millisecond)
    : m_mutex(mutex), m_is_locked(false) {
        timedlock(_millisecond);
    }

    ~BaseScopedLock() {
        if (m_is_locked) {
            unlock();
        }
    }

    bool isLocked() {
        return m_is_locked;
    }

    void lock() {

        if (!m_is_locked && m_mutex.lock()) {
            m_is_locked = true;
        }
    }

    void unlock() {
        if (m_is_locked) {
            m_mutex.unlock();
            m_is_locked = false;
        }
    }

    bool tryLock() {
        if (m_is_locked) {
            return false;
        }
        m_is_locked = m_mutex.trylock();
        return m_is_locked;
    }

#ifdef __linux__
    bool timedlock(long _millisecond) {
        if (m_is_locked) {
            return true;
        }
        m_is_locked = m_mutex.timedlock(_millisecond);
        return m_is_locked;
    }
#else
    bool timedlock(long _millisecond) {
        if (m_is_locked) {
            return true;
        }

        unsigned long start = gettickcount();
        unsigned long cur = start;

        while(cur < start + _millisecond) {
            if (trylock()) {
                break;
            }
            usleep(50 * 1000);
            cu = gettickcount();
        }
        return m_is_locked;
    }
#endif

    MutexType& internal() {
        return m_mutex;
    }

private:
    MutexType& m_mutex;
    volatile bool m_is_locked;
};


typedef BaseScopedLock<Mutex> ScopedLock;
typedef BaseScopedLock<SpinLock> ScopedSpinLock;

#endif // _LOCK_H
