//
// Created by 张德文 on 2022/5/24.
//

#include "log_zstd_buffer.h"

LogZstdBuffer::LogZstdBuffer(void *pbuffer, size_t len, bool isCompress, const char *pubkey,
                             void *_pbuffer, size_t _len, bool _is_compress,
                             const char *_pubkey, int level) : LogBaseBuffer(pbuffer, len, isCompress, pubkey) {

}

LogZstdBuffer::~LogZstdBuffer() noexcept {

}

size_t LogZstdBuffer::Compress(const void* src, size_t _inLen, void* dst, size_t _outLen) {

}

void LogZstdBuffer::Flush(AutoBuffer& _buf) {

}

bool LogZstdBuffer::__Reset() {

}
char LogZstdBuffer::__GetMagicSyncStart() {

}
char LogZstdBuffer::__GetMagicAsyncStart() {

}

