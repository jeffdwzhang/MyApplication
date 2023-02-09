//
// Created by 张德文 on 2021/11/14.
//

#include "ptr_buffer.h"

#define LOG_TAG "PtrBuffer"
#include "LogUtil.h"

const PtrBuffer KNullPtrBuffer(nullptr, 0, 0);

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

PtrBuffer::PtrBuffer(void* _ptr, size_t _len, size_t _max_len)
    : m_parray((unsigned char*)_ptr), m_pos(0), m_length(_len), m_max_length(_max_len) {

}

PtrBuffer::PtrBuffer(void* _ptr, size_t _len)
    : m_parray((unsigned char*)_ptr), m_pos(0), m_length(_len), m_max_length(_len) {

}

PtrBuffer::PtrBuffer() {
    Reset();
}

PtrBuffer::~PtrBuffer() {

}

void PtrBuffer::Write(const void* _pBuffer, size_t _nLen) {
    Write(_pBuffer, _nLen, Pos());
    Seek(_nLen, kSeekCur);
}

void PtrBuffer::Write(const void* _pBuffer, size_t _nLen, off_t _nPos) {
    size_t copyLen = min(_nLen, m_max_length - _nPos);
    m_length = max(m_length, copyLen + _nPos);
    memcpy((unsigned char*)Ptr() + _nPos, _pBuffer, copyLen);
}

size_t PtrBuffer::Read(void* _pBuffer, size_t _nLen) {
    size_t nRead = Read(_pBuffer, _nLen, Pos());
    Seek(nRead, kSeekCur);
    return nRead;
}

size_t PtrBuffer::Read(void* _pBuffer, size_t _nLen, off_t _nPos) const {

    size_t nRead = Length() - _nPos;
    nRead = min(nRead, _nLen);
    memcpy(_pBuffer, PosPtr(), nRead);
    return nRead;
}

void PtrBuffer::Seek(off_t _nOffset, TSeek _eOrigin) {
    switch (_eOrigin) {

        case kSeekStart:
            m_pos = _nOffset;
            break;
        case kSeekCur:
            m_pos += _nOffset;
            break;
        case kSeekEnd:
            m_pos = m_length + _nOffset;
            break;
        default:
//            ASSERT(false);
            break;
    }

    if (m_pos < 0) {
        m_pos = 0;
    }

    if (m_pos > m_length) {
        LOGW("pos[%ld] is larger than length[%zu]", m_pos, m_length);
        m_pos = m_length;
    }

}

void PtrBuffer::Length(off_t _nPos, size_t _nLength) {

    m_length = m_max_length < _nLength ? m_max_length : _nLength;
    Seek(_nPos, kSeekStart);
}

void* PtrBuffer::Ptr() {
    return m_parray;
}

const void* PtrBuffer::Ptr() const {
    return m_parray;
}

void* PtrBuffer::PosPtr() {
    return ((unsigned char*)Ptr()) + Pos();
}

const void* PtrBuffer::PosPtr() const {
    return ((unsigned char*)Ptr()) + Pos();
}

size_t PtrBuffer::Pos() const {
    return m_pos;
}

size_t PtrBuffer::PosLength() const {
    return m_length - m_pos;
}

size_t PtrBuffer::Length() const {
    return m_length;
}

size_t PtrBuffer::MaxLength() const {
    return m_max_length;
}

void PtrBuffer::Attach(void* _pBuffer, size_t _nLen, size_t _maxLen) {
    LOGD("Attach");
    Reset();
    m_parray = (unsigned char*)_pBuffer;
    m_length = _nLen;
    m_max_length = _maxLen;
}
void PtrBuffer::Attach(void* _pBuffer, size_t _nLen) {
    Attach(_pBuffer, _nLen, _nLen);
}
void PtrBuffer::Reset() {
    m_parray = nullptr;
    m_pos = 0;
    m_length = 0;
    m_max_length = 0;
}

