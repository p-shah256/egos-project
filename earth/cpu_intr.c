/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: RISC-V interrupt and exception handling
 *
 * Updated by CS6640 23fall staff
 */

#include "egos.h"

/* These is a static variable storing
 * the addresse of the trap handler function;
 * Initially, the variable is NULL */
static void (*trap_handler)(unsigned int);

/* Register handler functions by modifying the static variables */
int trap_handler_register(void (*_handler)(unsigned int)) { trap_handler = _handler; }

void trap_entry()  __attribute__((interrupt ("machine"), aligned(128)));
void trap_entry() {
    m_uint32 mcause;
    asm("csrr %0, mcause" : "=r"(mcause));

    if (!trap_handler) {
        FATAL("trap_entry: trap handler not registered (mcause=%x)", mcause);
    }

    trap_handler(mcause);
}

int intr_enable() {
    int mstatus_val, mie_val;
    asm("csrr %0, mstatus" : "=r"(mstatus_val));
    asm("csrw mstatus, %0" ::"r"(mstatus_val | 0x8));
    asm("csrr %0, mie" : "=r"(mie_val));
    /* For now, egos-2k+ only uses timer and software interrupts */
    asm("csrw mie, %0" ::"r"(mie_val | 0x88));
}

void intr_init() {
    INFO("Use direct mode and put the address of trap_entry() to mtvec");
    asm("csrw mtvec, %0" ::"r"(trap_entry));

    earth->intr_enable = intr_enable;
    earth->trap_handler_register = trap_handler_register;
}
