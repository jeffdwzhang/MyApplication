//
// Created by 张德文 on 2022/7/26.
//

#ifndef MYAPPLICATION_ALOGBASE_H
#define MYAPPLICATION_ALOGBASE_H

#include <string>
#include <vector>
#include <sys/time.h>
#include <time.h>
#include <stdarg.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef enum {
        kLevelAll = 0,
        kLevelVerbose = 0,
        kLevelDebug,
        kLevelInfo,
        kLevelWarn,
        kLevelError,
        kLevelFatal,
        kLevelNone
    } ALogLevel;

    enum AppenderMode {
        kAppenderAsync,  // 异步
        kAppenderSync    // 同步
    };

    enum CompressMode {
        kZlib,
        kZstd
    };

    struct ALogConfig {
        AppenderMode mode = kAppenderAsync;
        std::string logdir;
        std::string cachedir;
        std::string nameprefix;
        std::string pub_key;
        CompressMode compress_mode = kZlib;
        int compress_level;
        int cache_day = 0;
    };

    typedef struct ALoggerInfo_t {
        ALogLevel level;
        const char* tag;
        struct timeval timeval;
        intmax_t pid;
        intmax_t tid;
        intmax_t maintid;
        int traceLog;
    } LoggerInfo;

    extern intmax_t logger_pid();
    extern intmax_t logger_tid();
    extern intmax_t logger_maintid();

#ifdef __cplusplus
};
#endif

#endif //MYAPPLICATION_ALOGBASE_H
