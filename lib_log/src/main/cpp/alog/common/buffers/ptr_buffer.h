//
// Created by 张德文 on 2021/11/14.
//

#ifndef ANDROIDAVLEARN_PTR_BUFFER_H
#define ANDROIDAVLEARN_PTR_BUFFER_H

#include <sys/types.h>
#include <string.h>

class PtrBuffer {
public:
    enum TSeek {
        kSeekStart,
        kSeekCur,
        kSeekEnd
    };

public:
    PtrBuffer(void* _ptr, size_t _len, size_t _max_len);
    PtrBuffer(void* _ptr, size_t _len);
    PtrBuffer();
    ~PtrBuffer();

    template<class T> void Write(const T& _val) {
        Wite(&_val, sizeof(_val), m_pos);
    }

    template<class T> void Write(int _nPos, const T& _val) {
        Write(&_val, sizeof(_val), _nPos);
    }

    void Write(const char* const _val) {
        Write(_val, (unsigned int) strlen(_val));
    }

    void Write(const void* _pBuffer, size_t _nLen);

    void Write(const void* _pBuffer, size_t _nLen, off_t _nPos);

    template<class T> void Read(T& _val) {
        Read(&_val, sizeof(_val));
    }

    template<class T> void Read(int _nPos, const T& _val) const {
        Read(&_val, sizeof(_val), _nPos);
    }

    size_t Read(void* _pBuffer, size_t _nLen);
    size_t Read(void* _pBuffer, size_t _nLen, off_t _nPos) const;

    void Seek(off_t _nOffset, TSeek _eOrigin = kSeekCur);
    void Length(off_t _nPos, size_t _nLength);

    void* Ptr();
    void* PosPtr();
    const void* Ptr() const;
    const void* PosPtr() const;

    size_t Pos() const;
    size_t PosLength() const;
    size_t getLength() const;
    size_t MaxLength() const;

    void Attach(void* _pBuffer, size_t _nLen, size_t _maxLen);
    void Attach(void* _pBuffer, size_t _nLen);
    void Reset();

private:

    PtrBuffer(const PtrBuffer& _rhs);
    PtrBuffer& operator=(const PtrBuffer& _rhs);

    unsigned char* m_parray;
    off_t m_pos;
    size_t m_length;
    size_t m_max_length;
};

extern const PtrBuffer kNullPtrBuffer;

#endif //ANDROIDAVLEARN_PTR_BUFFER_H
