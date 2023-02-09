//
// Created by 张德文 on 2022/7/26.
//

#include "ALogCategory.h"
#include "LogUtil.h"
#include "thread.h"

ALogCategory* ALogCategory::NewInstance(uintptr_t _appender,
                                              std::function<void(const LoggerInfo *,
                                                                 const char *)> _appender_func) {
    return new ALogCategory(_appender, _appender_func);
}

void ALogCategory::DelayRelease(ALogCategory *_category) {
    Thread([_category] { return __Release(_category); }).start_after(5000);
}

void ALogCategory::__Release(ALogCategory *_category) {
    delete _category;
}

ALogCategory::ALogCategory(uintptr_t _appender,
                                 std::function<void (const LoggerInfo *,
                                                     const char *)> _appender_func)
        : m_appender(_appender), m_appender_func(_appender_func){

}

ALogCategory::~ALogCategory() {

}

uintptr_t ALogCategory::GetAppender() {
    return m_appender;
}

ALogLevel ALogCategory::GetLevel() {
    LOGD("GetLevel -> m_level:%d", m_level);
    return m_level;
}

void ALogCategory::SetLevel(ALogLevel _level) {
    m_level = _level;
}

bool ALogCategory::IsEnabledFor(ALogLevel _level) {
    return _level <= m_level;
}

void ALogCategory::Write(const LoggerInfo *_info, const char *_log) {

    if (nullptr == _info || nullptr == _log) {
        // 如果没有log，那么就不输出呗
        return;
    }
    __WriteImpl(_info, _log);
}

void ALogCategory::__WriteImpl(const LoggerInfo *_info, const char *_log) {
    if (nullptr == m_appender_func) {
        return;
    }

    if (-1 == _info->pid && -1 == _info->tid && -1 == _info->maintid) {
        LoggerInfo* info = (LoggerInfo*)_info;
        info->pid = logger_pid();
        info->tid = logger_tid();
        info->maintid = logger_maintid();
    }

    // 输出log
    m_appender_func(_info, _log);
}
