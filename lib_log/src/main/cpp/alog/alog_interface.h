//
// Created by 张德文 on 2022/7/26.
//

#ifndef MYAPPLICATION_ALOG_INTERFACE_H
#define MYAPPLICATION_ALOG_INTERFACE_H

#include <stdint.h>
#include "ALogBase.h"
#include "ALogCategory.h"

ALogCategory* NewLoggerInstance(const ALogConfig& _config, ALogLevel _level);

ALogCategory* GetLoggerInstance(const char* _name_prefix);

void RelaseLoggerInstance(uintptr_t _instance_ptr);

void LoggerWrite(uintptr_t _instance_ptr, const LoggerInfo* _info, const char* _log);

bool IsEnabledFor(uintptr_t _instance_ptr, ALogLevel _level);

ALogLevel GetLevel(uintptr_t _instance_ptr);

void SetLevel(uintptr_t _instance_ptr, ALogLevel _level);

void SetAppenderMode(uintptr_t _instance_ptr, AppenderMode _mode);

void Flush(uintptr_t _instance_ptr);

void SetConsoleLogOpen(uintptr_t _instance_ptr, bool _is_open);

void SetMaxFileSize(uintptr_t _instance_ptr, long _max_size);

void SetMaxAliveTime(uintptr_t _instance_ptr, long _alive_time);

void appender_open(const ALogConfig& _config, ALogLevel _level);
void appender_close();
void appender_flush();

#endif //MYAPPLICATION_ALOG_INTERFACE_H
