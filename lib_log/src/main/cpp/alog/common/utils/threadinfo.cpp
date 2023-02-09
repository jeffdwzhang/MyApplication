//
// Created by 张德文 on 2022/10/30.
//

#include <unistd.h>
#include "compiler_util.h"

extern "C"
{
EXPORT_FUNC intmax_t logger_pid()
{
    static intmax_t pid = getpid();
    return pid;
}

EXPORT_FUNC intmax_t logger_tid()
{
    return gettid();
}

EXPORT_FUNC intmax_t logger_maintid()
{
    static intmax_t pid = getpid();
    return pid;
}
}