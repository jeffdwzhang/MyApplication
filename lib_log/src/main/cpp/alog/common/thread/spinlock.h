//
// Created by 张德文 on 2022/4/22.
//

#ifndef ANDROIDAVLEARN_SPINLOCK_H
#define ANDROIDAVLEARN_SPINLOCK_H


static inline void cpu_relax() {

#if defined(__arc__) || defined(__mips__) || defined(__arm__) || defined(__powerpc__)
    asm volatile("" ::: "memory");
#elif defined(__i386__) || defined(__x86_64__)
    asm volatile("rep; nop" ::: "memory");
#elif defined(__aarch64__)
    asm volatile("yield" ::: "memory");
#elif defined(__ia64__)
    asm volatile ("hint @pause" ::: "memory");
#endif

}

#include <sched.h>
#include "atomic_oper.h"

/**
 * 自旋锁
 */
class SpinLock {

public:
    typedef uint32_t handle_type;

    SpinLock() : m_state(0) {}

    bool trylock() {
        return (atomic_cas32((volatile uint32_t *)&m_state, 1, 0) == 0);
    }

    bool lock() {

        return true;
    }

    bool unlock() {
        atomic_write32((volatile uint32_t *)&m_state, 0);
        return true;
    }

    uint32_t* internal() {
        return &m_state;
    }

private:
    enum state {
        initial_pause =2,
        max_pause = 16
    };

    uint32_t m_state;

private:
    // 禁止拷贝构造
    SpinLock(const SpinLock&);
    SpinLock& operator = (const SpinLock&);
};


#endif //ANDROIDAVLEARN_SPINLOCK_H
