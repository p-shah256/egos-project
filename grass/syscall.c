/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: the system call interface to user applications
 *
 * Updated by CS6640 23fall staff
 */

/* syscall workflow:

sys_send/recv (syscall.c)
  |                                                         ^
  +-> sys_invoke (syscall.c)                              [ret]
        |                              USER SPACE           |
-----[trap]-------------------------------------------------|----
        |                               KERNEL              |
        +-> trap_entry (cpu_intr.c)                       [mret]
            |                                               |
            +-> trap_handler (kernel.c)                   [ret]
                [switch to kernel stack]          [switch to user stack]
                ...                                         |
                |                                           |
                +-> proc_syscall (syscall.c)              [ret]
                    |                                       |
                    +-> proc_send/recv (syscall.c)        [ret]
                        |                                   |
                        +-> sys_yield (scheduler.c)       [ret]
                            |                               |
                            +->[context switch]       [switch back]
                                  |                         |
                                  +-->[other proc running]--+
 */


#include "egos.h"
#include "syscall.h"
#include "process.h"
#include <string.h>


/* Syscall first half running in user-space */
static void sys_invoke() {
    struct syscall *sc = (struct syscall*)SYSCALL_ARG;

#ifndef ECALL
    /* set machine-mode software interrupt:
     * MSIP bit of mip to 1 */
    *((int*)0x2000000) = 1;
    while (sc->type != SYS_UNUSED);
#else
    /* [lab4-ex1]
     * TODO: use ecall to trap to kerenl */

#endif
}

void sys_yield() {
    struct syscall *sc = (struct syscall*)SYSCALL_ARG;
    sc->type = SYS_YIELD;
    sys_invoke();
}

int sys_send(int receiver, char* msg, int size) {
    if (size > SYSCALL_MSG_LEN) return -1;

    struct syscall *sc = (struct syscall*)SYSCALL_ARG;
    sc->type = SYS_SEND;
    sc->msg.receiver = receiver;
    memcpy(sc->msg.content, msg, size);
    sys_invoke();
    return sc->retval;
}

int sys_recv(int* sender, char* buf, int size) {
    if (size > SYSCALL_MSG_LEN) return -1;

    struct syscall *sc = (struct syscall*)SYSCALL_ARG;
    sc->type = SYS_RECV;
    if (*sender > 0) {
        ASSERTX(*sender < MAX_NPROCESS);
        // the process has an expected sender
        sc->msg.sender = *sender;
    } else {
        // waiting for any sender
        sc->msg.sender = 0;
    }
    sys_invoke();
    memcpy(buf, sc->msg.content, size);
    if (sender) *sender = sc->msg.sender;
    return sc->retval;
}

void sys_exit(int status) {
    struct proc_request req;
    req.type = PROC_EXIT;
    sys_send(GPID_PROCESS, (void*)&req, sizeof(req));
}

void sys_sleep(int time_units) {
    struct proc_request req;
    req.type = PROC_SLEEP;
    req.argc = 1;
    *(int*)req.argv = time_units;
    sys_send(GPID_PROCESS, (void*)&req, sizeof(req));
    // FIXME: a hacky-way of implementing sleep
    // ASSUMPTION: the GPID_PROC runs at least once before the sys_yield() returns
    sys_yield();
}

/* Syscall second half running in kernel-space */
static void proc_send(struct syscall *sc);
static void proc_recv(struct syscall *sc);
void msgcpy(int src_pid, int dst_pid, void* src_addr, void* dst_addr, int size);

void proc_syscall() {
    struct syscall *sc = (struct syscall*)SYSCALL_ARG;

#ifdef VMON
    // copy syscall args from process address space to kernel space
    msgcpy(curr_pid, 0/*kernel*/, sc, sc, sizeof(struct syscall));
#endif

    int type = sc->type;
    sc->retval = 0;

#ifndef ECALL
    sc->type = SYS_UNUSED;
    /* set machine-mode software interrupt:
     * MSIP bit of mip to 0 */
    *((int*)0x2000000) = 0;
#endif

    switch (type) {
    case SYS_RECV:
        proc_recv(sc);
        break;
    case SYS_SEND:
        proc_send(sc);
        break;
    case SYS_YIELD:
        proc_yield();
        break;
    default:
        FATAL("proc_syscall: got unknown syscall type=%d", type);
    }
}

void msgcpy(int src_pid, int dst_pid, void* src_addr, void* dst_addr, int size) {
    ASSERTX(size > 0 && size < PAGE_SIZE);

#if VMON
    /* [lab5-ex3]
     * TODO: copy the message from src_pid to dst_pid.
     * hint: earth->mmu_translate() will be useful*/

#else
    char buf[size];
    /* Copy message from src to a temp buffer*/
    earth->mmu_switch(src_pid);
    memcpy(buf, src_addr, size);
    /* Copy message from the temp buffer to dst*/
    earth->mmu_switch(dst_pid);
    memcpy(dst_addr, buf, size);
#endif
}


static void proc_send(struct syscall *sc) {
    sc->msg.sender = curr_pid;
#ifdef VMON
    struct syscall *u_sc = earth->mmu_translate(curr_pid,sc);
    u_sc->msg.sender = curr_pid; // set the sender in the user address space
#endif

    /* Find the receiver */
    int receiver = sc->msg.receiver;
    int recv_idx = pid2idx(receiver);
    ASSERTX(proc_set[recv_idx].pid == receiver);

    /* if receiver status is not waiting OR
     *    the receiver is waiting for another pid:
     *    wait_to_send */
    if (proc_set[recv_idx].status != PROC_WAIT_TO_RECV ||
          (proc_set[recv_idx].from_sender_pid != curr_pid &&
           proc_set[recv_idx].from_sender_pid != 0) )
    {
        curr_status = PROC_WAIT_TO_SEND;
        proc_set[proc_curr_idx].to_receiver_pid = receiver;
    } else {
        /* Copy message from sender to kernel buf, then to receiver*/
        msgcpy(curr_pid, receiver, &sc->msg, &sc->msg,\
                sizeof(struct sys_msg));

        /* Set receiver process as runnable */
        proc_set_runnable(receiver);
    }

    proc_yield();
}

static void proc_recv(struct syscall *sc) {
    int exp_sender = sc->msg.sender;
    ASSERTX(exp_sender >= 0 && exp_sender < MAX_NPROCESS);
    proc_set[proc_curr_idx].from_sender_pid = exp_sender; // [0, MAX_NPROCESS)

    int sender = -1;
    if (exp_sender == 0) {  // search for any valid sender
        for (int i = 0; i < MAX_NPROCESS; i++)
            if (proc_set[i].status == PROC_WAIT_TO_SEND &&
                proc_set[i].to_receiver_pid == curr_pid)
                sender = proc_set[i].pid;
    } else { // target one sender
        if (proc_set[pid2idx(exp_sender)].status == PROC_WAIT_TO_SEND &&
            proc_set[pid2idx(exp_sender)].to_receiver_pid == curr_pid)
                sender = exp_sender;
    }

    if (sender == -1) {
        curr_status = PROC_WAIT_TO_RECV;
    } else {
        /* Copy message from sender to kernel buf, then to receiver*/
        msgcpy(sender, curr_pid, &sc->msg, &sc->msg,\
                sizeof(struct sys_msg));

        /* Set sender process as runnable */
        proc_set_runnable(sender);
    }

    proc_yield();
}

