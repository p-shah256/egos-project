#pragma once

#include "elf.h"
#include "disk.h"

enum {
    PROC_UNUSED,
    PROC_LOADING, /* allocated and wait for loading elf binary */
    PROC_READY,   /* finished loading elf and wait for first running */
    PROC_RUNNING,
    PROC_RUNNABLE,
    PROC_WAIT_TO_SEND,
    PROC_WAIT_TO_RECV,
    PROC_SLEEPING,
};

struct process{
    int pid;
    int status;
    int from_sender_pid; /* used when waiting to receive a message */
    int to_receiver_pid; /* used when waiting to send a message */
    void *sp, *mepc;  /* process context = stack pointer (sp)
                       * + machine exception program counter (mepc) */
    // scheduling attributes
    union {
        unsigned char      chars[64];
        unsigned int       ints[16];
        float              floats[16];
        unsigned long long longlongs[8];
        double             doubles[8];
    } schd_attr;
};

void timer_init();
void timer_reset();

int  proc_alloc();
void proc_free(int);
void proc_sleep(int, int);
void proc_set_ready (int);
void proc_set_running (int);
void proc_set_runnable (int);

void ctx_entry(void);
void ctx_start(void** old_sp, void* new_sp);
void ctx_switch(void** old_sp, void* new_sp);


/* defined in kernel.c */
extern int proc_curr_idx;
extern struct process proc_set[MAX_NPROCESS];
#define curr_pid      proc_set[proc_curr_idx].pid
#define curr_status   proc_set[proc_curr_idx].status

void trap_handler(); // defined in kernel.c
void proc_yield();   // defined in scheduler.c
void proc_syscall(); // defined in syscall.c
void proc_on_arrive(int pid); // defined in scheduler.c
void proc_on_sleep(int pid, int time_units);  // defined in scheduler.c
void proc_on_stop(int pid);   // defined in scheduler.c
