//
// Created by 张德文 on 2022/5/24.
//

#ifndef ANDROIDAVLEARN_LOG_ZLIB_BUFFER_H
#define ANDROIDAVLEARN_LOG_ZLIB_BUFFER_H

#include <string.h>
#include <stdint.h>
#include <zlib.h>

#include "log_base_buffer.h"
#include "ptr_buffer.h"
#include "autobuffer.h"

class LogCrypt;
class LogZlibBuffer : public LogBaseBuffer {
public:
    LogZlibBuffer(void* _pbuffer, size_t _len, bool _is_compress, const char* _pubkey);
    virtual ~LogZlibBuffer();

public:
    virtual size_t Compress(const void* src, size_t _inLen, void* dst, size_t _outLen);
    virtual void Flush(AutoBuffer& _buf);

private:
    bool __Reset();
    char __GetMagicSyncStart();
    char __GetMagicAsyncStart();

private:
    z_stream  m_z_stream;
};
#endif //ANDROIDAVLEARN_LOG_ZLIB_BUFFER_H
