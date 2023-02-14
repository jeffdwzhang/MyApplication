//
// Created by 张德文 on 2022/8/17.
//

#include "log_crypt.h"

#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "log_magic_num.h"

#define LOG_TAG "LogCrypt"
#include "LogUtil.h"

#define ALOG_NO_CRYPT

#ifndef ALOG_NO_CRYPT
#include "uECC.h"
#endif

const static int TEA_BLOCK_LEN = 8;

static void __TeaEncrypt(uint32_t* v, uint32_t* k) {
    uint32_t v0 = v[0];
    uint32_t v1 = v[1];
    uint32_t sum = 0;
    uint32_t i;

    const static uint32_t delta = 0x9e3779b9;
    uint32_t k0 = k[0];
    uint32_t k1 = k[1];
    uint32_t k2 = k[2];
    uint32_t k3 = k[3];

    for (i = 0; i < 16; i++) {
        sum += delta;
        v0 += ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1);
        v1 += ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3);
    }
    v[0] = v0;
    v[1] = v1;
}

static uint16_t __GetSeq(bool _is_async) {
    if (!_is_async) {
        return 0;
    }

    static uint16_t s_seq = 0;
    s_seq++;
    if (0 == s_seq) {
        s_seq++;
    }
    return s_seq;
}

#ifndef ALOG_NO_CRYPT
static bool Hex2Buffer(const char* _str, size_t _len, unsigned char* _buffer) {
    if (nullptr == _str || 0 == _len || 0 != (_len % 2)) {
        return false;
    }

    char temp[3] = {0};
    for (size_t i = 0; i < _len-1; i+=2) {
        for (size_t j = 0; j < 2; ++j) {
            temp[j] = _str[i + j];
            if (!(('0' <= temp[j] && temp[j] <= '9')
            || ('a' <= temp[j] && temp[j] <= 'f')
            || ('A' <= temp[j] && temp[j] <= 'F'))) {
                return false;
            }
        }
        _buffer[i/2] = (unsigned char) strtol(temp, nullptr, 16);
    }

    return true;
}
#endif

LogCrypt::LogCrypt(const char *_pubkey) : m_seq(0), m_is_crypt(false) {
#ifndef ALOG_NO_CRYPT
    const static size_t PUB_KEY_LEN = 64;
    if (nullptr == _pubkey || PUB_KEY_LEN * 2 != strnlen(_pubkey, 256)) {
        return;
    }

    unsigned char svr_pubkey[PUB_KEY_LEN] = {0};
    if (!Hex2Buffer(_pubkey, PUB_KEY_LEN * 2, svr_pubkey)) {
        return;
    }

    uint8_t client_pri[32] = {0};
    if (0 == uECC_make_key((uint8_t*)m_client_pubkey, client_pri, uECC_secp256k1())) {
        return;
    }

    uint8_t ecdh_key[32] = {0};
    if (0 == uECC_shared_secret(svr_pubkey, client_pri, ecdh_key, uECC_secp256k1())) {
        return;
    }

    memcpy(m_tea_key, ecdh_key, sizeof(m_tea_key));
    m_is_crypt = true;
#endif
}

LogCrypt::~LogCrypt() {

}

LogCrypt::LogCrypt(const LogCrypt&) {

}

LogCrypt& LogCrypt::operator=(const LogCrypt&) {

}

/**
 * |magic start(char)|seq(uint16_t)|begin hour(char)|end hour(char)|length(uint32_t)|crypt key(char*64)|
 */
uint32_t LogCrypt::GetHeaderLen() {
    return sizeof(char) * 3 + sizeof(uint16_t) + sizeof(uint32_t) + sizeof(char) * 64;
}

uint32_t LogCrypt::GetTailerLen() {
    return sizeof(char);
}

bool LogCrypt::GetLogHour(const char* const _data, size_t _len, int& _begin_hour, int& _end_hour) {
    if (_len < GetHeaderLen()) {
        return false;
    }

    char start = _data[0];
    if (!LogMagicNum::MagicStartIsValid(start)) {
        return false;
    }

    char begin_hour = _data[sizeof(char) + sizeof(uint16_t)];
    char end_hour = _data[sizeof(char) + sizeof(uint16_t) + sizeof(char)];

    _begin_hour = (int)begin_hour;
    _end_hour = (int)end_hour;
    LOGD("GetLogHour -> begin hour:%d, end hour:%d", _begin_hour, _end_hour);
    return true;
}

void LogCrypt::UpdateLogHour(char* _data) {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    time_t sec = tv.tv_sec;
    struct tm tm_temp = *localtime((const time_t*)&sec);
    char hour = (char)tm_temp.tm_hour;
//    LOGD("UpdateLogHour -> hour:%d", hour);
    memcpy(_data + GetHeaderLen() - sizeof(uint32_t) - sizeof(char) * 64 - sizeof(char), &hour,sizeof(hour));
}

uint32_t LogCrypt::GetLogLen(const char *const _data, size_t _len) {
    if (_len < GetHeaderLen()) return 0;

    char start = _data[0];
    LOGD("GetLogLen -> start:%d",start);
    if (!LogMagicNum::MagicStartIsValid(start)) {
        LOGW("GetLogLen -> not valid start:%d",start);
        return 0;
    }

    uint32_t len = 0;
    memcpy(&len, _data + GetHeaderLen() - sizeof(uint32_t) - sizeof(char) * 64, sizeof(len));
//    LOGD("GetLogLen -> len:%d",len);
    return len;
}

void LogCrypt::UpdateLogLen(char *_data, uint32_t _add_len) {
    uint32_t currentLen = (uint32_t)(GetLogLen(_data, GetHeaderLen()) + _add_len);
    LOGD("UpdateLogLen -> UpdateLogLen:%d",currentLen);
    memcpy(_data + GetHeaderLen() - sizeof(uint32_t) - sizeof(char)*64, &currentLen, sizeof(currentLen));
}

void LogCrypt::SetHeaderInfo(char *_data, bool _is_async, char _magic_start) {
    memcpy(_data, &_magic_start, sizeof(_magic_start));
    m_seq = __GetSeq(_is_async);
    memcpy(_data + sizeof(_magic_start), &m_seq, sizeof(m_seq));

    struct timeval tv;
    gettimeofday(&tv, 0);
    time_t sec = tv.tv_sec;
    tm tm_temp = *localtime((const time_t*)&sec);
    char hour = (char )tm_temp.tm_hour;
    memcpy(_data + sizeof(_magic_start) + sizeof(m_seq), &hour, sizeof(hour));
    memcpy(_data + sizeof(_magic_start) + sizeof(m_seq) + sizeof(hour), &hour, sizeof(hour));

    uint32_t len = 0;
    memcpy(_data + sizeof(_magic_start) + sizeof(m_seq) + sizeof(hour) * 2, &len, sizeof(len));
    memcpy(_data + sizeof(_magic_start) + sizeof(m_seq) + sizeof(hour) * 2 + sizeof(len), m_client_pubkey, sizeof(m_client_pubkey));
}

void LogCrypt::SetTailerInfo(char *_data, char _magic_end) {
    memcpy(_data, &_magic_end, sizeof(_magic_end));
}

void LogCrypt::CryptSyncLog(const char *const _log_data,
                            size_t _inputLen,
                            AutoBuffer &_out_buff,
                            char _magic_start,
                            char _magic_end) {

    _out_buff.AllocWrite(GetHeaderLen() + GetTailerLen() + _inputLen);

    // 写入头标记
    SetHeaderInfo((char*)_out_buff.Ptr(), false, _magic_start);

    uint32_t header_len = GetHeaderLen();
    UpdateLogLen((char *)_out_buff.Ptr(), (uint32_t)_inputLen);

    // 写入尾标记
    SetTailerInfo((char *)_out_buff.Ptr() + _inputLen + header_len, _magic_end);

    // 写入log内容
    memcpy((char *)_out_buff.Ptr() + header_len, _log_data, _inputLen);

#ifndef ALOG_NO_CRYPT

#endif
}

void LogCrypt::CryptASyncLog(const char *const _log_data,
                             size_t _inputLen,
                             std::string &_out_buff,
                             size_t &_remain_nocrypt_len) {
    if (!m_is_crypt) {
        _out_buff.assign(_log_data, _inputLen);
        _remain_nocrypt_len = 0;
        return;
    }

#ifndef ALOG_NO_CRYPT
    uint32_t temp[2] = {0};
    size_t count = _inputLen / TEA_BLOCK_LEN;
    _remain_nocrypt_len = _inputLen % TEA_BLOCK_LEN;
    for (size_t i = 0; i < count; ++i) {
        memcpy(temp, _log_data + i * TEA_BLOCK_LEN, TEA_BLOCK_LEN);
        __TeaEncrypt(temp, m_tea_key);
        _out_buff.append((char *)temp, TEA_BLOCK_LEN);
    }
    _out_buff.append(_log_data + _inputLen - _remain_nocrypt_len, _remain_nocrypt_len);
#endif


}

bool LogCrypt::Fix(char *_data, size_t _data_len, uint32_t &_raw_log_len) {
    if (_data_len < GetHeaderLen()) {
        return false;
    }

    char start = _data[0];
    if (!LogMagicNum::MagicStartIsValid(start)) {
        return false;
    }

    _raw_log_len = GetLogLen(_data, _data_len);

    memcpy(&m_seq, _data + sizeof(char), sizeof(m_seq));
    return true;
}

bool LogCrypt::IsCrypt() {
    return m_is_crypt;
}