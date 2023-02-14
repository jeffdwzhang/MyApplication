//
// Created by 张德文 on 2021/11/14.
//

#include "log_base_buffer.h"

#include <cstdio>
#include <ctime>
#include <sys/time.h>
#include <algorithm>
#include <cstring>
#include <errno.h>
#include <cassert>
#include <zlib.h>
#include "crypt/log_magic_num.h"

#define LOG_TAG "LogBaseBuffer"
#include "LogUtil.h"

/**
 * 获取指定时间段内的log
 * @param _log_path
 * @param _begin_hour
 * @param _end_hour
 * @param _begin_pos
 * @param _end_pos
 * @param _err_msg
 * @return
 */
bool LogBaseBuffer::GetPeriodLogs(const char *_log_path,
                                  int _begin_hour,
                                  int _end_hour,
                                  unsigned long &_begin_pos,
                                  unsigned long &_end_pos,
                                  std::string &_err_msg) {

    char msg[1024] = {0};
    char magic_end = LogMagicNum::kMagicEnd;

    if (nullptr == _log_path || _end_hour <= _begin_hour) {
        snprintf(msg, sizeof(msg), "_log_path is null or end hour(%d) <= begin hour(%d)", _end_hour, _begin_hour);
        _err_msg += msg;
        return false;
    }

    FILE *file = fopen(_log_path, "rb");
    if (nullptr == file) {
        snprintf(msg, sizeof(msg), "open file fail:%s", strerror(errno));
        _err_msg += msg;
        return false;
    }

    if (0 != fseek(file, 0, SEEK_END)) {
        snprintf(msg, sizeof(msg), "file seek fail:%s", strerror(ferror(file)));
        _err_msg += msg;
        fclose(file);
        return false;
    }

    long file_size = ftell(file);
    if (0 != fseek(file, 0, SEEK_SET)) {
        snprintf(msg, sizeof(msg), "file seek fail:%s", strerror(ferror(file)));
        _err_msg += msg;
        fclose(file);
        return false;
    }



}

LogBaseBuffer::LogBaseBuffer(void* _pbuffer, size_t _len, bool _is_compress, const char* _pubkey)
: m_is_compress(_is_compress), m_log_crypt(new LogCrypt(_pubkey)), m_is_crypt(m_log_crypt->IsCrypt()) {
    m_buffer.Attach(_pbuffer, _len);
    __Fix();
}

LogBaseBuffer::~LogBaseBuffer() {
    delete m_log_crypt;
}

PtrBuffer& LogBaseBuffer::GetData() {
    return m_buffer;
}

void LogBaseBuffer::Flush(AutoBuffer& _buffer) {
    if (m_log_crypt->GetLogLen((char*)m_buffer.Ptr(), m_buffer.getLength()) == 0) {
        __Clear();
        return;
    }

    __Flush();
    LOGD("Flush -> buffer length:%zu", m_buffer.getLength());
    _buffer.Write(m_buffer.Ptr(), m_buffer.getLength());
    __Clear();
}

bool LogBaseBuffer::Write(const void* _data, size_t _length, AutoBuffer& _out_buffer) {
    if (nullptr == _data || 0 == _length) {
        return false;
    }

    m_log_crypt->CryptSyncLog((char *)_data, _length, _out_buffer, __GetMagicSyncStart(), __GetMagicEnd());
    return true;
}

bool LogBaseBuffer::Write(const void* _data, size_t _length) {

    if (nullptr == _data || 0 == _length) {
        return false;
    }

    if (m_buffer.getLength() == 0) {
        if (!__Reset()) {
            return false;
        }
    }

    size_t before_len = m_buffer.getLength();
    size_t write_len = _length;

    if (m_is_compress) {
        uInt avail_out = (uInt)(m_buffer.getMaxLength() - m_buffer.getLength());

        write_len = Compress(_data, _length, m_buffer.PosPtr(), avail_out);
        LOGD("Write -> after compress:%zu", write_len);
        if (write_len == (size_t)-1) {
            return false;
        }
    } else {
        m_buffer.Write(_data, _length);
    }

    before_len -= m_remain_no_crypt_len;

    std::string out_buffer;
    size_t last_remain_len = m_remain_no_crypt_len;

    m_log_crypt->CryptASyncLog((char*)m_buffer.Ptr() + before_len, write_len + m_remain_no_crypt_len, out_buffer, m_remain_no_crypt_len);

    m_buffer.Write(out_buffer.data(), out_buffer.size(), before_len);

    before_len += out_buffer.size();
    m_buffer.Length(before_len, before_len);

    m_log_crypt->UpdateLogLen((char*)m_buffer.Ptr(), (uint32_t)(out_buffer.size() - last_remain_len));

    return true;
}

bool LogBaseBuffer::__Reset() {
    LOGD("__Reset -> __Clear");
    __Clear();
    LOGD("__Reset -> SetHeaderInfo");
    m_log_crypt->SetHeaderInfo((char*)m_buffer.Ptr(), m_is_compress, __GetMagicAsyncStart());
    m_buffer.Length(m_log_crypt->GetHeaderLen(), m_log_crypt->GetHeaderLen());

    return true;
}

void LogBaseBuffer::__Flush() {
//    LOGD("flush");
    m_log_crypt->UpdateLogHour((char*)m_buffer.Ptr());

    m_log_crypt->SetTailerInfo((char*)m_buffer.Ptr() + m_buffer.getLength(), __GetMagicEnd());
    m_buffer.Length(m_buffer.getLength() + m_log_crypt->GetTailerLen(),
                    m_buffer.getLength() + m_log_crypt->GetTailerLen());
}

void LogBaseBuffer::__Clear() {
    LOGD("__Clear");
    memset(m_buffer.Ptr(), 0, m_buffer.getMaxLength());
    m_buffer.Length(0,0);
    m_remain_no_crypt_len = 0;
}

void LogBaseBuffer::__Fix() {
//    LOGD("fix");
    uint32_t raw_log_len = 0;
    if (m_log_crypt->Fix((char*)m_buffer.Ptr(), m_buffer.getLength(), raw_log_len)) {
        m_buffer.Length(raw_log_len + m_log_crypt->GetHeaderLen(), raw_log_len + m_log_crypt->GetHeaderLen());
    } else {
        m_buffer.Length(0, 0);
    }
}

char LogBaseBuffer::__GetMagicEnd() {
    return LogMagicNum::kMagicEnd;
}



