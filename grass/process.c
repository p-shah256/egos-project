/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: helper functions for managing processes
 *
 * Updated by CS6640 23fall staff
 */

#include "egos.h"
#include "process.h"
#include "syscall.h"
#include <string.h>

static void proc_set_status(int pid, int status) {
    ASSERTX(pid == proc_set[pid2idx(pid)].pid);
    proc_set[pid2idx(pid)].status = status;
}

int proc_alloc() {
    for (int i = 0; i < MAX_NPROCESS; i++)
        if (proc_set[i].status == PROC_UNUSED) {
            proc_set[i].pid = idx2pid(i);
            proc_set[i].status = PROC_LOADING;

            /* notify scheduler that a process has arrived */
            int pid = proc_set[i].pid;
            proc_on_arrive(pid);

            return pid;
        }

    FATAL("proc_alloc: reach the limit of %d processes", MAX_NPROCESS);
}

void proc_free(int pid) {
    if (pid != -1) {
        earth->mmu_free(pid);
        proc_set_status(pid, PROC_UNUSED);
        /* notify scheduler that a process has stopped */
        proc_on_stop(pid);
        return;
    }

    /* Free all user applications */
    for (int i = 0; i < MAX_NPROCESS; i++)
        if (proc_set[i].pid >= GPID_USER_START &&
            proc_set[i].status != PROC_UNUSED) {
            earth->mmu_free(proc_set[i].pid);
            proc_set[i].status = PROC_UNUSED;
            proc_on_stop(proc_set[i].pid); // notify scheduler
        }
}

void proc_sleep(int pid, int time_units) {
    /* notify scheduler that a process wants to sleep */
    proc_on_sleep(pid, time_units);
}

void proc_set_ready(int pid) { proc_set_status(pid, PROC_READY); }
void proc_set_running(int pid) { proc_set_status(pid, PROC_RUNNING); }
void proc_set_runnable(int pid) { proc_set_status(pid, PROC_RUNNABLE); }
