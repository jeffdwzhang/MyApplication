//
// Created by 张德文 on 2021/11/14.
//

#ifndef _LOG_BASE_BUFFER_H
#define _LOG_BASE_BUFFER_H

#include "ptr_buffer.h"
#include "autobuffer.h"
#include "log_crypt.h"

class LogBaseBuffer {
public:
    LogBaseBuffer(void* _pbuffer, size_t _len, bool _is_compress, const char* _pubkey);
    virtual ~LogBaseBuffer();

public:
    static bool GetPeriodLogs(const char* _log_path, int _begin_hour, int _end_hour, unsigned long& _begin_pos, unsigned long& _end_pos, std::string& _err_msg);

public:
    PtrBuffer& GetData();
    virtual size_t Compress(const void* src, size_t _inLen, void* dst, size_t _outLen) = 0;
    virtual void Flush(AutoBuffer& _buf);
    bool Write(const void* _data, size_t _length);
    bool Write(const void* _data, size_t _length, AutoBuffer& _outBuf);

protected:
    virtual bool __Reset();
    void __Flush();
    void __Clear();
    void __Fix();
    char __GetMagicEnd();
    virtual char __GetMagicSyncStart() = 0;
    virtual char __GetMagicAsyncStart() = 0;

protected:
    PtrBuffer m_buffer;
    bool m_is_compress;
    class LogCrypt* m_log_crypt;
    bool m_is_crypt;
    size_t m_remain_no_crypt_len;

};

#endif // _LOG_BASE_BUFFER_H
