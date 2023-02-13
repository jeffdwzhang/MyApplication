//
// Created by 张德文 on 2022/8/20.
//

#include "ALogBase.h"
#include "ptr_buffer.h"
#include "extract.h"
#include "LogUtil.h"

void log_formater(const LoggerInfo* _info, const char* _log, PtrBuffer& _log_buffer) {

    static const char* levelStrings[] = {
            "V",
            "D",
            "I",
            "W",
            "E",
            "F"
    };

    static int error_count = 0;
    static int error_size = 0;
//    LOGD("log_formater -> max length:%zu, length:%zu", _log_buffer.MaxLength(), _log_buffer.getLength());
    if (_log_buffer.MaxLength() <= _log_buffer.getLength() + 5 * 1024) {
        ++error_count;
        error_size = (int) strnlen(_log, 1024 * 1024);

        if (_log_buffer.MaxLength() >= _log_buffer.getLength() + 128) {
            int ret = snprintf((char *)_log_buffer.PosPtr(), 1024, "[F]log_size <= 5*1024, err(%d, %d)\n", error_count, error_size);
            _log_buffer.Length(_log_buffer.Pos() + ret, _log_buffer.getLength() + ret);
            _log_buffer.Write("");

            error_count = 0;
            error_size = 0;
        }
        return;
    }

    if (nullptr != _info) {

        // 获取时间戳
        char temp_time[64] = {0};
        if (0 != _info->timeval.tv_sec) {
            const time_t sec = _info->timeval.tv_sec;
            tm tm = *localtime(&sec);

#ifdef ANDROID
            snprintf(temp_time, sizeof(temp_time), "%d-%02d-%02d %+.1f %02d:%02d:%02d.%.3ld", 1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday,
                     tm.tm_gmtoff / 3600.0, tm.tm_hour, tm.tm_min, tm.tm_sec, _info->timeval.tv_usec / 1000);
#else
            snprintf(temp_time, sizeof(temp_time), "%d-%02d-%02d %+.1f %02d:%02d:%02d.%.3d", 1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday,
                 tm.tm_gmtoff / 3600.0, tm.tm_hour, tm.tm_min, tm.tm_sec, _info->timeval.tv_usec / 1000);
#endif
        }

//        LOGD("log_formater -> temp time:%s", temp_time);

        int ret = snprintf((char *)_log_buffer.PosPtr(), 1024, "[%s][%s][%jd, %jd%s][%s]",
                           _log ? levelStrings[_info->level] : levelStrings[kLevelFatal], temp_time,
                           _info->pid, _info->tid, _info->tid == _info->maintid ? "*" : "", _info->tag ? _info->tag : "");

        _log_buffer.Length(_log_buffer.Pos() + ret, _log_buffer.getLength() + ret);

    }

    if (nullptr != _log) {

        size_t bodyLen = _log_buffer.MaxLength() - _log_buffer.getLength() > 130 ? _log_buffer.MaxLength() -
                                                                                   _log_buffer.getLength() - 130 : 0;
//        LOGD("log_formater -> bodyLen:%zu", bodyLen);
        bodyLen = bodyLen > 0xFFFFU ? 0xFFFFU : bodyLen;
        bodyLen = strnlen(_log, bodyLen);
        bodyLen = bodyLen > 0xFFFFU ? 0xFFFFU : bodyLen;
        _log_buffer.Write(_log, bodyLen);
    } else {
        _log_buffer.Write("error!!! log is null");
    }

    char nextline = '\n';
    if (*((char*)_log_buffer.PosPtr() -1) != nextline) { // 如果最后一个字符不是换行符，那么就补上一个换行符
        _log_buffer.Write(&nextline, 1);
    }
}

