//
// Created by 张德文 on 2022/4/22.
//

#ifndef ANDROIDAVLEARN_MUTEX_H
#define ANDROIDAVLEARN_MUTEX_H

#include <errno.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/time.h>

class Mutex {

public:
    typedef pthread_mutex_t handle_type;
    Mutex(bool _recursive = false) : m_magic(), m_mutex(), m_mutexattr() {

        // 禁止重复加锁
        int ret = pthread_mutexattr_init(&m_mutexattr);


        ret = pthread_mutexattr_settype(&m_mutexattr,
                                        _recursive ? PTHREAD_MUTEX_RECURSIVE : PTHREAD_MUTEX_ERRORCHECK);

        ret = pthread_mutex_init(&m_mutex, &m_mutexattr);

    }

    ~Mutex() {
        m_magic = 0;

        int ret = pthread_mutex_destroy(&m_mutex);

        ret = pthread_mutexattr_destroy(&m_mutexattr);
    }

    bool lock() {

        if (reinterpret_cast<uintptr_t>(this) != m_magic) {
            return false;
        }

        int ret = pthread_mutex_lock(&m_mutex);

        return 0==ret;
    }

    bool unlock() {

        int ret = pthread_mutex_unlock(&m_mutex);

        return 0==ret;

    }

    bool trylock() {

        if (reinterpret_cast<uintptr_t>(this) != m_magic) {
            return false;
        }

        int ret = pthread_mutex_trylock(&m_mutex);

        if (EBUSY == ret) {
            return false;
        }

        return 0==ret;
    }

    bool timedlock(long _millisecond) {

        if (reinterpret_cast<uintptr_t>(this) != m_magic) {
            return false;
        }

        int ret = 0;
#if defined(ANDROID) && __ANDROID_API__ < 21 && !defined(__LP64__)
        ret = pthread_mute_locak_timeout_np(&m_mutex, (unsigned)_millisecond);
#else
        struct timespec ts;
        MakeTimeout(&ts, _millisecond);
        ret = pthread_mutex_timedlock(&m_mutex, &ts);
#endif
        switch (ret) {
            case 0: return true;
            case ETIMEDOUT: return false;
            case EBUSY: return false;
        }

        return false;
    }


    pthread_mutex_t& internal() {
        return m_mutex;
    }


private:
    Mutex(const Mutex&);
    Mutex& operator = (const Mutex&);

    static void MakeTimeout(struct timespec* pts, long _millisecond) {
        struct timeval tv;
        gettimeofday(&tv, 0);
        pts->tv_sec = _millisecond / 1000 + tv.tv_sec;
        pts->tv_nsec = (_millisecond % 1000) * 1000 * 1000 + tv.tv_usec * 1000;

        pts->tv_sec += pts->tv_nsec / (1000 * 1000 * 1000);
        pts->tv_nsec = pts->tv_nsec % (1000 * 1000 * 1000);
    }

private:
    uintptr_t m_magic;  // Dangling pointer will dead lock, so check it!!!
    pthread_mutex_t m_mutex;
    pthread_mutexattr_t m_mutexattr;
};

#endif //ANDROIDAVLEARN_MUTEX_H
