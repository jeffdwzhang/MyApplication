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
    // TODO


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
    if (m_log_crypt->GetLogLen((char*)m_buffer.Ptr(), m_buffer.Length()) == 0) {
        __Clear();
        return;
    }

    __Flush();
    LOGD("Flush -> buffer length:%zu", m_buffer.Length());
    _buffer.Write(m_buffer.Ptr(), m_buffer.Length());
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

    if (m_buffer.Length() == 0) {
        if (!__Reset()) {
            return false;
        }
    }

    size_t before_len = m_buffer.Length();
    size_t write_len = _length;

    if (m_is_compress) {

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
    __Clear();

    m_log_crypt->SetHeaderInfo((char*)m_buffer.Ptr(), m_is_compress, __GetMagicAsyncStart());
    m_buffer.Length(m_log_crypt->GetHeaderLen(), m_log_crypt->GetHeaderLen());

    return true;
}

void LogBaseBuffer::__Flush() {
    LOGD("flush");
    m_log_crypt->UpdateLogHour((char*)m_buffer.Ptr());
    m_log_crypt->SetTailerInfo((char*)m_buffer.Ptr() + m_buffer.Length(), __GetMagicEnd());
    m_buffer.Length(m_buffer.Length() + m_log_crypt->GetTailerLen(), m_buffer.Length() + m_log_crypt->GetTailerLen());
}

void LogBaseBuffer::__Clear() {
    LOGD("__Clear");
    memset(m_buffer.Ptr(), 0, m_buffer.MaxLength());
    m_buffer.Length(0,0);
    m_remain_no_crypt_len = 0;
}

void LogBaseBuffer::__Fix() {
    LOGD("fix");
    uint32_t raw_log_len = 0;
    if (m_log_crypt->Fix((char*)m_buffer.Ptr(), m_buffer.Length(), raw_log_len)) {
        m_buffer.Length(raw_log_len + m_log_crypt->GetHeaderLen(), raw_log_len + m_log_crypt->GetHeaderLen());
    } else {
        m_buffer.Length(0, 0);
    }
}

char LogBaseBuffer::__GetMagicEnd() {
    return LogMagicNum::kMagicEnd;
}



