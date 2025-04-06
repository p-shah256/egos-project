/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: timer reset and initialization
 * mtime is at 0x200bff8 and mtimecmp is at 0x2004000 in the memory map
 * see section 3.1.15 of references/riscv-privileged-v1.10.pdf
 * and section 9.1, 9.3 of references/sifive-fe310-v19p04.pdf
 *
 * updated by CS6640 23fall staff
 */
#include "egos.h"

static unsigned long long mtime_get() {
    unsigned int low, high;
    /* Q: Why having a loop? */
    do {
        high = *(unsigned int*)(0x200bff8 + 4);
        low  = *(unsigned int*)(0x200bff8);
    }  while ( *(unsigned int*)(0x200bff8 + 4) != high );

    return (((unsigned long long)high) << 32) | (unsigned long long)low;
}

/* set "mtimecmp" to "time" */
static void mtimecmp_set(unsigned long long time) {
    /* Q: Why setting mtimecmp low to all 0xF? */
    *(unsigned int*)(0x2004000 + 4) = 0xFFFFFFFF;
    *(unsigned int*)(0x2004000 + 0) = (unsigned int)time;
    *(unsigned int*)(0x2004000 + 4) = (unsigned int)(time >> 32);
}

void timer_reset() {
    mtimecmp_set(mtime_get() + QUANTUM);
}

m_uint64 gettime() {
    static m_uint64 last_time = 0;
    m_uint64 time = (m_uint64) mtime_get();
    if (time < last_time) {
        INFO("gettime() overflows");
    }
    last_time = time;
    return time;
}

void timer_init() {
    earth->timer_reset = timer_reset;
    earth->gettime = gettime;
}
