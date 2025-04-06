/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: interrupt and exception handler
 *
 * Updated by CS6640 23fall staff.
 */


#include "egos.h"
#include "process.h"
#include "syscall.h"
#include <string.h>

#define INTR_ID_SOFT       3
#define INTR_ID_TIMER      7

/* Kernel status */
int proc_curr_idx;
struct process proc_set[MAX_NPROCESS];

void intr_entry(int id) {
    if (id == INTR_ID_TIMER && curr_pid < GPID_SHELL) {
        /* Do not interrupt kernel processes since IO can be stateful */
        earth->timer_reset();
        return;
    }

    if (curr_pid >= GPID_USER_START && earth->tty_intr()) {
        /* User process killed by ctrl+c interrupt */
        INFO("process %d killed by interrupt", curr_pid);
        proc_set[proc_curr_idx].mepc = (void*) (APPS_ENTRY + 0xC);
        return;
    }

    if (id == INTR_ID_SOFT) {
        proc_syscall();
    } else if (id == INTR_ID_TIMER) {
        proc_yield();
    } else
        FATAL("intr_entry: got unknown interrupt %d", id);
}

void excp_entry(int id) {
    /*
     * [lab4-ex2]
     * TODO: handling exceptions
     * - If "id" is for syscalls, handle the system call and return:
     *   -- you need to capture **all** ecall exceptions
     *   -- you need to think of which pc the CPU will run after "mret";
     *      in other word, you need to properly update PCBs so that
     *      eventually "mepc" will be set to the right instruction.
     * - If "id" is not for syscalls, then check
     *   -- if curr_pid is a user application, kill the process
     *   -- if curr_pid is a system proc, panic the kernel using FATAL
     */


    FATAL("fatal exception (pid=%d) %d", curr_pid, id);
}

void check_nested_trap() {
    unsigned int sp;
    asm("mv %0, sp" : "=r"(sp));
    if ((GRASS_STACK_TOP - sp) < 2 * PAGE_SIZE) {
        FATAL("trap_handler: meet nested trap in kernel");
    }
}

static unsigned int trap_cause;
void trap_handler(unsigned int mcause) {
    check_nested_trap();

    // save mcause for later use
    trap_cause = mcause;

    /* Switch to the kernel stack */
    ctx_start(&proc_set[proc_curr_idx].sp, (void*)GRASS_STACK_TOP);
    // jump to ctx_entry
}


void manage_userstack(int save) {
    // FIXME later:
    // a better approach is to use the original process's stack
    // but for now, they are somewhere unknown. To do so, we need to walk the page table
    static struct {char data[PAGE_SIZE];} proc_stack[MAX_NPROCESS];

    int stack_size = APPS_STACK_TOP - (m_uint32)proc_set[proc_curr_idx].sp;
    ASSERTX(stack_size <= PAGE_SIZE);
    if (save) {  // save the current stack to tmp stack arr
        memcpy(&proc_stack[curr_pid], proc_set[proc_curr_idx].sp, stack_size);
    } else {  // restore pid's stack from the tmp stack arr
        memcpy(proc_set[proc_curr_idx].sp, &proc_stack[curr_pid], stack_size);
    }
}


void ctx_entry() {
    /* Now on the kernel stack */
    int mepc, ignore;
    asm("csrr %0, mepc" : "=r"(mepc));
    proc_set[proc_curr_idx].mepc = (void*) mepc;

    // save the current proc's user stack
    manage_userstack(1/*save user stack*/);

    /* handle either interrupt or exception */
    int id = trap_cause & 0x3FF;
    (trap_cause & (1 << 31)) ? intr_entry(id) : excp_entry(id);

    // restore user stack of the chosen proc
    manage_userstack(0/*restore user stack*/);

    /* [lab4-ex3]
     * TODO: the kernel will switch privilege level here:
     * - if the curr_pid is a system process, set the privilege level to S-Mode
     * - if the curr_pid is a user application, set the privilege level to U-Mode
     */

    /* TODO: your code here */


    /* Switch back to the user application stack */
    mepc = (int)proc_set[proc_curr_idx].mepc;
    asm("csrw mepc, %0" ::"r"(mepc));
    ctx_switch((void**)&ignore, proc_set[proc_curr_idx].sp);
}

