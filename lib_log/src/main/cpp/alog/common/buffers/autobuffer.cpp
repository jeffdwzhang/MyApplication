//
// Created by 张德文 on 2022/5/23.
//

#include "autobuffer.h"
#include <stdint.h>
#include <stdlib.h>

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

AutoBuffer::AutoBuffer(size_t _size)
    : m_parray(nullptr), m_pos(0), m_length(0), m_capacity(0), m_malloc_unit_size(_size) {

}

AutoBuffer::AutoBuffer(void* _pbuffer, size_t _len, size_t _size)
        : m_parray(nullptr), m_pos(0), m_length(0), m_capacity(0), m_malloc_unit_size(_size) {
    Attach(_pbuffer, _len);
}

AutoBuffer::AutoBuffer(const void* _pbuffer, size_t _len, size_t _size)
        : m_parray(nullptr), m_pos(0), m_length(0), m_capacity(0), m_malloc_unit_size(_size) {
    Write(0, _pbuffer, _len);
}

AutoBuffer::~AutoBuffer() {
    Reset();
}

void AutoBuffer::AllocWrite(size_t _readytowrite, bool _changelength) {
    size_t nLen = Pos() + _readytowrite;
    __FitSize(nLen);

    if (_changelength) {
        m_length = max(nLen, m_length);
    }
}

void AutoBuffer::AddCapacity(size_t _len) {
    __FitSize(Capacity() + _len);
}

void AutoBuffer::Write(const AutoBuffer& _buffer) {
    Write(_buffer.Ptr(), _buffer.Length());
}

void AutoBuffer::Write(const void* _pbuffer, size_t _len) {
    Write(Pos(), _pbuffer, _len);
    Seek(_len, ESeekCur);
}

void AutoBuffer::Write(off_t& _pos, const AutoBuffer& _buffer) {
    Write((const off_t&)_pos, _buffer.Ptr(), _buffer.Length());
    _pos += _buffer.Length();
}

void AutoBuffer::Write(off_t& _pos, const void* _pbuffer, size_t _len) {
    Write((const off_t&)_pos, _pbuffer, _len);
    _pos += _len;
}

void AutoBuffer::Write(const off_t& _pos, const AutoBuffer& _buffer) {
    Write((const off_t&)_pos, _buffer.Ptr(), _buffer.Length());
}

void AutoBuffer::Write(const off_t& _pos, const void* _pbuffer, size_t _len) {
    size_t nLen = _pos + _len;
    __FitSize(nLen);
    m_length = max(nLen, m_length);
    memcpy((unsigned char*)Ptr() + _pos, _pbuffer, _len);
}

void AutoBuffer::Seek(off_t _offset, TSeek _eorigin) {
    switch (_eorigin) {
        case ESeekStart:
            m_pos = _offset;
            break;
        case ESeekCur:
            m_pos += _offset;
            break;
        case ESeekEnd:
            m_pos = m_length + _offset;
            break;
        default:
            break;
    }

    if (m_pos < 0) {
        m_pos = 0;
    }

    if ((size_t)m_pos > m_length) {
        m_pos = m_length;
    }
}

void AutoBuffer::Length(off_t _pos, size_t _length) {
    m_length = _length;
    Seek(_pos, ESeekStart);
}

void* AutoBuffer::Ptr(off_t _offset) {
    return (unsigned char*)m_parray + _offset;
}

const void* AutoBuffer::Ptr(off_t _offset) const {
    return (const unsigned char*)m_parray + _offset;
}

void* AutoBuffer::PosPtr() {
    return (unsigned char*)m_parray + Pos();
}

const void* AutoBuffer::PosPtr() const {
    return (const unsigned char*)m_parray + Pos();
}

off_t AutoBuffer::Pos() const {
    return m_pos;
}

size_t AutoBuffer::PosLength() const {
    return m_length - m_pos;
}

size_t AutoBuffer::Length() const {
    return m_length;
}

size_t AutoBuffer::Capacity() const {
    return m_capacity;
}

void AutoBuffer::Attach(void *_pbuffer, size_t _len) {
    Reset();
    m_parray = (unsigned char*)_pbuffer;
    m_length = _len;
    m_capacity = _len;
}

void AutoBuffer::Attach(AutoBuffer& _rhs) {
    Reset();
    m_parray = _rhs.m_parray;
    m_pos = _rhs.m_pos;
    m_length= _rhs.m_length;
    m_capacity = _rhs.m_capacity;

    _rhs.m_parray = nullptr;
    _rhs.Reset();
}

void* AutoBuffer::Detach(size_t* _plen) {
    unsigned char* ret = m_parray;
    m_parray = nullptr;
    size_t nLen = Length();

    if (nullptr == _plen) {
        *_plen = nLen;
    }

    Reset();
    return ret;
}

void AutoBuffer::Reset() {
    if (nullptr != m_parray) {
        free(m_parray);
    }

    m_parray = nullptr;
    m_pos = 0;
    m_length = 0;
    m_capacity = 0;
}

void AutoBuffer::__FitSize(size_t _len) {
    if (_len > m_capacity) {
        //
        size_t mallocsize = ((_len + m_malloc_unit_size -1)/ m_malloc_unit_size) * m_malloc_unit_size;
        void* p = realloc(m_parray, mallocsize);

        if (nullptr == p) {
            free(m_parray);
            m_parray = nullptr;
            m_capacity = 0;
            return;
        }

        m_parray = (unsigned char*) p;

        memset(m_parray+m_capacity, 0, mallocsize-m_capacity);
        m_capacity = mallocsize;
    }
}

