//
// Created by 张德文 on 2022/7/26.
//

#include "alog_interface.h"

#include "ALogAppender.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "alog_interface"
#include "LogUtil.h"

// 缓存append模式下的底层实例，append模式下，java层无需持有log底层实例，由native层的引用指针来持有
static ALogCategory* sg_local_category = nullptr;

void appender_open(const ALogConfig& _config, ALogLevel _level) {
    LOGD("appender_open");
    sg_local_category = NewLoggerInstance(_config, _level);
}

void appender_close() {
    if (nullptr != sg_local_category) {
        LOGD("appender_close");
        auto* appender = reinterpret_cast<ALogAppender*>(sg_local_category->GetAppender());
        appender->Close();
        delete sg_local_category;
        sg_local_category = nullptr;
    }
}

void appender_flush() {
    if (nullptr != sg_local_category) {
        LOGD("appender_flush");
        auto* appender = reinterpret_cast<ALogAppender*>(sg_local_category->GetAppender());
        appender->Flush();
    }
}

/**
 * 新建一个log实例
 * @param _config
 * @param _level
 * @return
 */
ALogCategory* NewLoggerInstance(const ALogConfig& _config, ALogLevel _level) {
    // 检查对应的参数
    if (_config.logdir.empty()) {
        LOGE("log dir is empty, so ignore");
        return nullptr;
    }

    if (_config.nameprefix.empty()) {
        LOGE("name prefix is empty, so ignore");
        return nullptr;
    }

//    LOGD("NewLoggerInstance -> new ALogAppender instance");
    ALogAppender* appender = ALogAppender::NewInstance(_config);
//    LOGD("NewLoggerInstance -> new ALogCategory instance");
    ALogCategory* category = ALogCategory::NewInstance(reinterpret_cast<uintptr_t>(appender),
                                                             std::bind(&ALogAppender::Write, appender, std::placeholders::_1, std::placeholders::_2));
//    LOGD("NewLoggerInstance -> set level %d", _level);
    category->SetLevel(_level);

    return category;
}

/**
 * 释放对应的log实例
 * @param _instance_ptr
 */
void ReleaseLoggerInstance(uintptr_t _instance_ptr) {
    ALogCategory* category = reinterpret_cast<ALogCategory*>(_instance_ptr);
    delete category;
    return;
}

/**
 * 写log
 * @param _instance_ptr
 * @param _info
 * @param _log
 */
void LoggerWrite(uintptr_t _instance_ptr, const LoggerInfo* _info, const char* _log) {
    ALogCategory* category = sg_local_category;
    if (0 != _instance_ptr) {
        category = reinterpret_cast<ALogCategory*>(_instance_ptr);
    }
    if (nullptr != category) {
        category->Write(_info, _log);
    }
    return;
}

/**
 *
 * @param _instance_ptr
 * @param _level
 * @return
 */
bool IsEnabledFor(uintptr_t _instance_ptr, ALogLevel _level) {
    ALogCategory* category = sg_local_category;
    if (0 != _instance_ptr) {
        category = reinterpret_cast<ALogCategory*>(_instance_ptr);
    }
    if (nullptr != category) {
        return category->IsEnabledFor(_level);
    }
    return false;
}

ALogLevel GetLevel(uintptr_t _instance_ptr) {
    ALogCategory* category = sg_local_category;
    if (0 != _instance_ptr) {
        category = reinterpret_cast<ALogCategory*>(_instance_ptr);
    }
    if (nullptr != category) {
        return category->GetLevel();
    }
    return kLevelAll;
}

/**
 * 设置log输出级别
 *
 * @param _instance_ptr
 * @param _level
 */
void SetLevel(uintptr_t _instance_ptr, ALogLevel _level) {
    ALogCategory* category = sg_local_category;
    if (0 != _instance_ptr) {
        category = reinterpret_cast<ALogCategory*>(_instance_ptr);
    }
    if (nullptr != category) {
        category->SetLevel(_level);
    }
    return;
}

/**
 *
 * @param _instance_ptr
 * @param _mode
 */
void SetAppenderMode(uintptr_t _instance_ptr, AppenderMode _mode) {
    ALogCategory* category = sg_local_category;
    if (0 != _instance_ptr) {
        category = reinterpret_cast<ALogCategory*>(_instance_ptr);
    }
    if (nullptr != category) {
        auto* appender = reinterpret_cast<ALogAppender*>(category->GetAppender());
        appender->SetMode(_mode);
    }
    return;
}

void Flush(uintptr_t _instance_ptr, bool _is_sync) {
    ALogCategory* category = sg_local_category;
    if (0 != _instance_ptr) {
        category = reinterpret_cast<ALogCategory*>(_instance_ptr);
    }
    if (nullptr != category) {
        auto* appender = reinterpret_cast<ALogAppender*>(category->GetAppender());
        _is_sync ? appender->FlushSync() : appender->Flush();
    }
    return;
}

/**
 * 开启log的console输出
 *
 * @param _instance_ptr
 * @param _is_open
 */
void SetConsoleLogOpen(uintptr_t _instance_ptr, bool _is_open) {
    ALogCategory* category = sg_local_category;
    if (0 != _instance_ptr) {
        category = reinterpret_cast<ALogCategory*>(_instance_ptr);
    }
    if (nullptr != category) {
        auto* appender = reinterpret_cast<ALogAppender*>(category->GetAppender());
        appender->SetConsoleLog(_is_open);
    }
    return;
}

void SetMaxFileSize(uintptr_t _instance_ptr, long _max_size) {
    ALogCategory* category = sg_local_category;
    if (0 != _instance_ptr) {
        category = reinterpret_cast<ALogCategory*>(_instance_ptr);
    }
    if (nullptr != category) {
        auto* appender = reinterpret_cast<ALogAppender*>(category->GetAppender());
        appender->SetMaxFileSize(_max_size);
    }
    return;
}

void SetMaxAliveTime(uintptr_t _instance_ptr, long _alive_time) {
    ALogCategory* category = sg_local_category;
    if (0 != _instance_ptr) {
        category = reinterpret_cast<ALogCategory*>(_instance_ptr);
    }
    if (nullptr != category) {
        auto* appender = reinterpret_cast<ALogAppender*>(category->GetAppender());
        appender->SetMaxAliveDuration(_alive_time);
    }
    return;
}

