//
// Created by 张德文 on 2022/5/24.
//

#include "log_zlib_buffer.h"
#include <cstdio>
#include <sys/time.h>
#include <algorithm>
#include <cstring>

#include "crypt/log_crypt.h"
#include "crypt/log_magic_num.h"

#define LOG_TAG "LogZlibBuffer"
#include "LogUtil.h"

LogZlibBuffer::LogZlibBuffer(void *_pbuffer, size_t _len, bool _is_compress, const char *_pubkey)
    : LogBaseBuffer(_pbuffer, _len, _is_compress, _pubkey) {
    if (m_is_compress) {
        memset(&m_z_stream, 0, sizeof(m_z_stream));
    }
}

LogZlibBuffer::~LogZlibBuffer() noexcept {
    if (m_is_compress && Z_NULL != m_z_stream.state) {
        deflateEnd(&m_z_stream);
    }
}

size_t LogZlibBuffer::Compress(const void *src, size_t _inLen, void *dst, size_t _outLen) {
    m_z_stream.avail_in = (uInt)_inLen;
    m_z_stream.next_in = (Bytef*)src;

    m_z_stream.next_out = (Bytef*)dst;
    m_z_stream.avail_out = (uInt)_outLen;

    if (Z_OK != deflate(&m_z_stream, Z_SYNC_FLUSH)) {
        return -1;
    }

    return _outLen - m_z_stream.avail_out;
}

void LogZlibBuffer::Flush(AutoBuffer& _buf) {
//    LOGD("flush");
    if (m_is_compress && Z_NULL != m_z_stream.state) {
        deflateEnd(&m_z_stream);
    }
    LogBaseBuffer::Flush(_buf);
}

bool LogZlibBuffer::__Reset() {
    if (!LogBaseBuffer::__Reset()) {
        return false;
    }

    if (m_is_compress) {
        m_z_stream.zalloc = Z_NULL;
        m_z_stream.zfree = Z_NULL;
        m_z_stream.opaque = Z_NULL;

        if (Z_OK != deflateInit2(&m_z_stream, Z_BEST_COMPRESSION, Z_DEFLATED, -MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY)) {
            return false;
        }
    }

    return true;
}

char LogZlibBuffer::__GetMagicSyncStart() {
    return m_is_crypt ? LogMagicNum::kMagicSyncZlibStart : LogMagicNum::kMagicSyncNoCryptZlibStart;
}
char LogZlibBuffer::__GetMagicAsyncStart() {
    return m_is_crypt ? LogMagicNum::kMagicAsyncZlibStart : LogMagicNum::kMagicAsyncNoCryptZlibStart;
}