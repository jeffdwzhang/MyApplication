//
// Created by 张德文 on 2022/8/16.
//

#include "tickcount.h"
#include "time_utils.h"

static uint64_t sg_tick_start = ::gettickcount();
static const uint64_t sg_tick_init = 2000000000;

tickcount_t::tickcount_t(bool _now): m_tickcount(0) {
    if (_now) {
        gettickcount();
    }
}

tickcount_t& tickcount_t::gettickcount() {
    m_tickcount = sg_tick_init + ::gettickcount() - sg_tick_start;
    return *this;
}
