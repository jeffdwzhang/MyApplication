//
// Created by 张德文 on 2022/5/24.
//

#include "log_zstd_buffer.h"

#include "crypt/log_crypt.h"
#include "crypt/log_magic_num.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "LogZstdBuffer"
#include "LogUtil.h"

/**
 * 使用zstd的流式压缩
 * @param pbuffer
 * @param len
 * @param is_compress
 * @param pubkey
 * @param level
 */
LogZstdBuffer::LogZstdBuffer(void *pbuffer, size_t len, bool is_compress, const char *pubkey, int level)
                             : LogBaseBuffer(pbuffer, len, is_compress, pubkey) {
    if (m_is_compress) {
        m_cctx = ZSTD_createCCtx();
        ZSTD_CCtx_setParameter(m_cctx, ZSTD_c_compressionLevel, level);
        ZSTD_CCtx_setParameter(m_cctx, ZSTD_c_windowLog, 16);
    }
}

LogZstdBuffer::~LogZstdBuffer() noexcept {
    if (m_is_compress && m_cctx != nullptr) {

        ZSTD_inBuffer input = {nullptr, 0, 0};
        ZSTD_outBuffer output = {nullptr, 0, 0};
        ZSTD_compressStream2(m_cctx, &output, &input, ZSTD_e_end);

        ZSTD_freeCCtx(m_cctx);
        m_cctx = nullptr;
    }
}

size_t LogZstdBuffer::Compress(const void* src, size_t _inLen, void* dst, size_t _outLen) {

    ZSTD_inBuffer input = {src, _inLen, 0};
    ZSTD_outBuffer output = {dst, _outLen, 0};
    size_t ret = ZSTD_compressStream2(m_cctx, &output, &input, ZSTD_e_flush);
    if (ret != 0 && ZSTD_isError(ret)) {
        LOGD("Compress -> ret:%zu, error name:%s", ret, ZSTD_getErrorName(ret));
    }
//    LOGD("Compress -> _inLen:%zu, _outLen:%zu, output.pos:%zu", _inLen, _outLen, output.pos);
    return output.pos;
}

void LogZstdBuffer::Flush(AutoBuffer& _buf) {
    if (m_is_compress && m_cctx != nullptr) {
        // 给一个压缩结束的标志
        ZSTD_inBuffer input = {nullptr, 0, 0};
        size_t ret = 0;
        do {

            size_t avail_out = m_buffer.getMaxLength() - m_buffer.getLength();
            ZSTD_outBuffer output = {m_buffer.PosPtr(), avail_out, 0};
            size_t beforeLen = m_buffer.Pos();
            ret = ZSTD_compressStream2(m_cctx, &output, &input, ZSTD_e_end);
            if (ret != 0 && ZSTD_isError(ret)) {
                LOGD("Flush -> ret:%d, error name:%s", ret, ZSTD_getErrorName(ret));
            }

            m_buffer.Length(beforeLen + output.pos, beforeLen + output.pos);

            m_log_crypt->UpdateLogLen((char*)m_buffer.Ptr(), output.pos);

        } while (ret != 0);
    }
    // 调用父类的Flush方法
    LogBaseBuffer::Flush(_buf);
}

bool LogZstdBuffer::__Reset() {
    if (!LogBaseBuffer::__Reset()) {
        return false;
    }
    if (m_is_compress) {
        LOGD("__Reset -> reset ZSTD_CCtx");
        ZSTD_CCtx_reset(m_cctx, ZSTD_reset_session_only);
    }

    return true;

}
char LogZstdBuffer::__GetMagicSyncStart() {
    return m_is_crypt ? LogMagicNum::kMagicSyncZstdStart : LogMagicNum::kMagicSyncNoCryptZstdStart;
}
char LogZstdBuffer::__GetMagicAsyncStart() {
    return m_is_crypt ? LogMagicNum::kMagicAsyncZstdStart : LogMagicNum::kMagicAsyncNoCryptZstdStart;
}

