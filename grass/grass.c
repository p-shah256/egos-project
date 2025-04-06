/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: grass layer initialization
 * Initialize timer and the process control block;
 * Spawn the first kernel process, GPID_PROCESS (pid=1).
 *
 * Updated by CS6640 23fall staff
 */

#include "egos.h"
#include "process.h"
#include "syscall.h"

struct grass *grass = (void*)GRASS_STRUCT_BASE;
struct earth *earth = (void*)EARTH_STRUCT_BASE;

static int sys_proc_read(int block_no, char* dst) {
    return earth->disk_read(SYS_PROC_EXEC_START + block_no, 1, dst);
}

int main() {
    CRITICAL("Enter the grass layer");

    /* register trap handler */
    earth->trap_handler_register(trap_handler);

    /* Initialize the grass interface functions */
    grass->proc_alloc = proc_alloc;
    grass->proc_sleep = proc_sleep;
    grass->proc_free = proc_free;
    grass->proc_set_ready = proc_set_ready;

    grass->sys_exit = sys_exit;
    grass->sys_yield = sys_yield;
    grass->sys_sleep = sys_sleep;
    grass->sys_send = sys_send;
    grass->sys_recv = sys_recv;

    /* Load and enter the first kernel process sys_proc */
    INFO("Load kernel process #%d: sys_proc", GPID_PROCESS);
    elf_load(GPID_PROCESS, sys_proc_read, 0, 0);
    SUCCESS("Successfully load process #%d: sys_proc", GPID_PROCESS);

    /* run the first process sys_proc*/
    earth->mmu_switch(GPID_PROCESS);
    proc_set_running(proc_alloc());

    /* jump to entry of the first process */
    void (*sys_proc_entry)() = (void*)APPS_ENTRY;
    asm("mv a0, %0" ::"r"(APPS_ARG));
    sys_proc_entry();
}
