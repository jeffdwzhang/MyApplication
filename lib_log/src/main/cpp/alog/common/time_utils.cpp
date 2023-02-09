//
// Created by 张德文 on 2022/10/4.
//

#include "time_utils.h"
#include <sys/time.h>
#include <stdint.h>

#ifdef ANDROID
#include <time.h>
#include <errno.h>
#include <linux/ioctl.h>
#include <stdatomic.h>
#include <fcntl.h>
#include <unistd.h>

#include "android_alarm.h"

uint64_t gettickcount() {
    static int s_fd = -1;
    static int errcode = 0;
    if (s_fd == -1 & EACCES != errcode) {
        int fd = open("/dev/alarm", O_RDONLY);
        if (-1 == fd) {
            errcode = errno;
        }
        atomic_int x = ATOMIC_VAR_INIT(s_fd);
        int expect = -1;
        if (!atomic_compare_exchange_strong(&x, &expect, fd)) {
            if (fd > 0) {
                close(fd);
            }
        }
        s_fd = atomic_load(&x);
    }

    struct timespec ts;
    int result = ioctl(s_fd, ANDROID_ALARM_GET_TIME(ANDROID_ALARM_ELAPSED_REALTIME), &ts);
    if (0 != result) {
        clock_gettime(CLOCK_BOOTTIME, &ts);
    }

    return (uint64_t)ts.tv_sec*1000 + (uint64_t)ts.tv_nsec/1000000;
}

uint64_t clock_app_monotonic() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (uint64_t)ts.tv_sec*1000 + (uint64_t)ts.tv_nsec/1000000;
}

#elif defined __linux__

#include <time.h>

uint64_t gettickcount() {
    struct timespec ts;
    if (0 == clock_gettime(CLOCK_MONOTONIC, &ts)) {
        return (ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000);
    }

    return 0;
}

uint64_t clock_app_monotonic() {
    return gettickcount();
}

#else
#error "not support"

#endif

uint64_t timeMs() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000;
}