//
// Created by 张德文 on 2022/5/23.
//

#ifndef _AUTOBUFFER_H
#define _AUTOBUFFER_H

#include <sys/types.h>
#include <string>

class AutoBuffer {
public:
    enum TSeek{
        ESeekStart,
        ESeekCur,
        ESeekEnd,
    };

public:
    explicit AutoBuffer(size_t _size = 128);
    explicit AutoBuffer(void* _pbuffer, size_t _len, size_t _size = 128);
    explicit AutoBuffer(const void* _pbuffer, size_t _len, size_t _size = 128);

    ~AutoBuffer();

    void AllocWrite(size_t _readytowrite, bool _changelength = true);
    void AddCapacity(size_t _len);

    template<class T> void Write(const T& _val) {
        Write(&_val, sizeof(_val));
    }

    void Write(const AutoBuffer& _buffer);
    void Write(const void* _pbuffer, size_t _len);
    void Write(off_t& _pos, const AutoBuffer& _buffer);
    void Write(off_t& _pos, const void* _pbuffer, size_t _len);
    void Write(const off_t& _pos, const AutoBuffer& _buffer);
    void Write(const off_t& _pos, const void* _pbuffer, size_t _len);


    void Seek(off_t _offset, TSeek _eorigin);
    void Length(off_t _pos, size_t _length);

    void* Ptr(off_t _offset = 0);
    void* PosPtr();
    const void* Ptr(off_t _offset=0) const;
    const void* PosPtr() const;

    off_t Pos() const;
    size_t PosLength() const;
    size_t Length() const;
    size_t Capacity() const;

    void Attach(void* pbuffer, size_t _len);
    void Attach(AutoBuffer& _rhs);
    void* Detach(size_t* _plen= nullptr);

    void Reset();

private:
    void __FitSize(size_t _len);

private:
    AutoBuffer(const AutoBuffer& _rhs);
    AutoBuffer& operator=(const AutoBuffer& _rhs);

private:
    unsigned char* m_parray;
    off_t m_pos;
    size_t m_length;
    size_t m_capacity;
    size_t m_malloc_unit_size;
};

#endif // _AUTOBUFFER_H
