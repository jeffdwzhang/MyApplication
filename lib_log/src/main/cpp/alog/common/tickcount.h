//
// Created by 张德文 on 2022/8/16.
//

#ifndef MYAPPLICATION_TICKCOUNT_H
#define MYAPPLICATION_TICKCOUNT_H

#include <stdint.h>

class tickcountdiff_t {
public:
    tickcountdiff_t(int64_t _diff) : m_tickcount_diff(_diff) {}
    operator int64_t() const { return m_tickcount_diff; }

    tickcountdiff_t& operator +=(int64_t _factor) { m_tickcount_diff += _factor; return *this; }
    tickcountdiff_t& operator -=(int64_t _factor) { m_tickcount_diff -= _factor; return *this; }
    tickcountdiff_t& operator *=(int64_t _factor) { m_tickcount_diff *= _factor; return *this; }

private:
    int64_t m_tickcount_diff;
};

class tickcount_t {
public:
    tickcount_t(bool _now = false);

    tickcountdiff_t operator-(const tickcount_t& _tc) const {
        return tickcountdiff_t(m_tickcount - _tc.m_tickcount);
    }

    uint64_t get() const { return m_tickcount; }
    tickcount_t& gettickcount();
    tickcountdiff_t gettickspan() const { return tickcount_t(true) - (*this); }


    bool operator< (const tickcount_t& _tc) const { return m_tickcount <  _tc.m_tickcount; }
    bool operator<=(const tickcount_t& _tc) const { return m_tickcount <= _tc.m_tickcount; }
    bool operator==(const tickcount_t& _tc) const { return m_tickcount == _tc.m_tickcount; }
    bool operator!=(const tickcount_t& _tc) const { return m_tickcount !=  _tc.m_tickcount; }
    bool operator> (const tickcount_t& _tc) const { return m_tickcount >  _tc.m_tickcount; }
    bool operator>=(const tickcount_t& _tc) const { return m_tickcount >=  _tc.m_tickcount; }

    bool isValid() {
        return 0 != m_tickcount;
    }

    void setInvalid() {
        m_tickcount = 0;
    }

private:
    uint64_t m_tickcount;

};

#endif //MYAPPLICATION_TICKCOUNT_H
