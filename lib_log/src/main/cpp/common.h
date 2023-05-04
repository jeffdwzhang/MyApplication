//
// Created by 张德文 on 2023/2/16.
//

#ifndef MYAPPLICATION_COMMON_H
#define MYAPPLICATION_COMMON_H

#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

#endif //MYAPPLICATION_COMMON_H
