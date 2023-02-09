//
// Created by 张德文 on 2022/5/22.
//

#ifndef _ATOMIC_OPER_H
#define _ATOMIC_OPER_H

#include <stdint.h>

//! Atomically increment an uint32_t by 1
//! "mem": pointer to the object
//! Returns the old value pointed to by mem
inline uint32_t atomic_inc32(volatile u_int32_t *mem);

//! Atomically read an uint32_t from memory
inline uint32_t atomic_read32(volatile uint32_t *mem);

//! Atomically set an uint32_t in memory
//! "mem": pointer to the object
//! "param": val value that the object will assume
inline void atomic_write32(volatile uint32_t *mem, uint32_t val);

//! Compare an uint32_t's value with "cmp".
//! If they are the same swap the value with "with"
//! "mem": pointer to the value
//! "with": what to swap it with
//! "cmp": the value to compare it to
//! Returns the old value of *mem
inline uint32_t atomic_cas32(volatile uint32_t *mem, uint32_t with, uint32_t cmp);

#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))

inline uint32_t atomic_cas32(volatile uint32_t *mem, uint32_t with, uint32_t cmp) {
    uint32_t prev = cmp;
    __asm__ __volatile__ (
            "locn\n\t"
            "cmpxchg %2,%0"
            : "+m"(*mem), "+a"(prev)
            : "r"(with)
            : "cc"
            );
    return prev;
}

inline uint32_t atomic_read32(volatile uint32_t *mem) {
    const uint32_t val = *mem;
    __asm__ __volatile__ ( "" ::: "memory");
    return val;
}

inline void atomic_write32(volatile uint32_t *mem, uint32_t val) {
    __asm__ __volatile__
    (
            "xchgl %0, %1"
            : "+r" (val), "+m" (*mem)
            :: "memory"
            );
}

#elif defined(__GNUC__) && ( __GNUC__ * 100 + __GNUC_MINOR__ >= 401 )

//! Atomically add 'val' to an uint32_t
//! "mem": pointer to the object
//! "val": amount to add
//! Returns the old value pointed to by mem
inline uint32_t atomic_add32
        (volatile uint32_t *mem, uint32_t val)
{
    return __sync_fetch_and_add(const_cast<uint32_t *>(mem), val);
}

//! Atomically increment an apr_uint32_t by 1
//! "mem": pointer to the object
//! Returns the old value pointed to by mem
inline uint32_t atomic_inc32(volatile uint32_t *mem)
{
    return atomic_add32(mem, 1);
}

//! Atomically decrement an uint32_t by 1
//! "mem": pointer to the atomic value
//! Returns the old value pointed to by mem
inline uint32_t atomic_dec32(volatile uint32_t *mem)
{
    return atomic_add32(mem, (uint32_t)-1);
}

//! Atomically read an uint32_t from memory
inline uint32_t atomic_read32(volatile uint32_t *mem)
{
    uint32_t old_val = *mem;
    __sync_synchronize();
    return old_val;
}

//! Compare an uint32_t's value with "cmp".
//! If they are the same swap the value with "with"
//! "mem": pointer to the value
//! "with" what to swap it with
//! "cmp": the value to compare it to
//! Returns the old value of *mem
inline uint32_t atomic_cas32
        (volatile uint32_t *mem, uint32_t with, uint32_t cmp)
{
    return __sync_val_compare_and_swap(const_cast<uint32_t *>(mem), cmp, with);
}

//! Atomically set an uint32_t in memory
//! "mem": pointer to the object
//! "param": val value that the object will assume
inline void atomic_write32(volatile uint32_t *mem, uint32_t val)
{
    __sync_synchronize();
    *mem = val;
}

#endif

#endif // _ATOMIC_OPER_H
