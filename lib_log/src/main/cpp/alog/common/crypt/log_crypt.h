//
// Created by 张德文 on 2022/8/17.
//

#ifndef MYAPPLICATION_LOG_CRYPT_H
#define MYAPPLICATION_LOG_CRYPT_H

#include <stdint.h>
#include <string>

#include "autobuffer.h"

class LogCrypt {
public:
    LogCrypt(const char* _pubkey);
    ~LogCrypt();

private:
    LogCrypt(const LogCrypt&);
    LogCrypt& operator=(const LogCrypt&);

public:
    static uint32_t GetHeaderLen();
    static uint32_t GetTailerLen();

    static bool GetLogHour(const char* const _data, size_t _len, int& _begin_hour, int& _end_hour);
    static void UpdateLogHour(char* _data);

    static uint32_t GetLogLen(const char* const _data, size_t _len);
    static void UpdateLogLen(char* _data, uint32_t _add_len);

public:
    void SetHeaderInfo(char* _data, bool _is_async, char _magic_start);
    void SetTailerInfo(char* _data, char _magic_end);

    void CryptSyncLog(const char* const _log_data, size_t _inputLen, AutoBuffer& _out_buff, char _magic_start, char _magic_end);
    void CryptASyncLog(const char* const _log_data, size_t _inputLen, std::string& _out_buff, size_t& _remain_nocrypt_len);

    bool Fix(char* _data, size_t _data_len, uint32_t& _raw_log_len);
    bool IsCrypt();

private:
    uint16_t m_seq;
    uint32_t m_tea_key[4];
    char m_client_pubkey[64] = {0};
    bool m_is_crypt;

};


#endif //MYAPPLICATION_LOG_CRYPT_H
