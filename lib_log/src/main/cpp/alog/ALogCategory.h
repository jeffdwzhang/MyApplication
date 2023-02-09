//
// Created by 张德文 on 2022/7/26.
//

#ifndef MYAPPLICATION_ALOGCATEGORY_H
#define MYAPPLICATION_ALOGCATEGORY_H

#include "ALogBase.h"

class ALogCategory {

public:
    static ALogCategory* NewInstance(uintptr_t _appender,
                                        std::function<void (const LoggerInfo* _info, const char* _log)> _appender_func);
    static void DelayRelease(ALogCategory* _category);

    ~ALogCategory();

    uintptr_t GetAppender();
    ALogLevel GetLevel();
    void SetLevel(ALogLevel _level);
    bool IsEnabledFor(ALogLevel _level);

    void Write(const LoggerInfo* _info, const char* _log);

private:

    ALogCategory(uintptr_t _appender,
    std::function<void (const LoggerInfo* _info, const char* _log)> _appender_func);
    static void __Release(ALogCategory* _category);

    void __WriteImpl(const LoggerInfo* _info, const char* _log);

    ALogLevel m_level = kLevelNone;
    uintptr_t m_appender = 0;
    std::function<void (const LoggerInfo* _info, const char* _log)> m_appender_func = nullptr;

};


#endif //MYAPPLICATION_ALOGCATEGORY_H
