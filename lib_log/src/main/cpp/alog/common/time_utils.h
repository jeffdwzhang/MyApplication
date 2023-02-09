//
// Created by 张德文 on 2022/10/4.
//

#ifndef MYAPPLICATION_TIME_UTILS_H
#define MYAPPLICATION_TIME_UTILS_H

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

    uint64_t gettickcount();  // ms
    int64_t gettickspan(uint64_t _ole_tick);   // ms
    uint64_t timeMs();

    uint64_t clock_app_monotonic();   // ms

#ifdef __cplusplus
};
#endif

#endif //MYAPPLICATION_TIME_UTILS_H
