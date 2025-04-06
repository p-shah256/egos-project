/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: scheduler
 *
 * updated by CS6640 23fall staff
 */

#include "egos.h"
#include "process.h"
#include <string.h>
#include "queue.h"

/* processes that have pid < USER_PID_START is system processes */
#define USER_PID_START 5
#define USER_IDX_START pid2idx(USER_PID_START)

/* [lab3-ex1]
 * TODO: plan how to use "schd_attr"
 *  - You need to design you own
 *  - Below are two examples.
 *  - With these macros, one can get the attribute by, for example,
 *       proc_set[pid2idx(pid)].arrive_time = earth->gettime();
 */
// #define arrive_time schd_attr.longlongs[0]
// #define schded_count schd_attr.ints[15]


static int mlfq();

/* schedule next process */
void proc_yield() {
    /* [lab3-ex1]
     * TODO: update "schd_attr" in "proc_set" for the current process
     * hints:
     *   - the current process will likely to be scheduled "out"
     *   - you can use earth->gettime() to get the current time
     *     -- the returned time is in "long long" (i.e., "m_uint64")
     *   - you may want to calculate how long it runs here
     *     -- a challenge is that the returned time may overflow
     * */




    /* Find the next runnable process */
    int next_idx = -1;

#ifndef MLFQ // if not defined MLFQ in Makefile
    /* default scheduler policy: find the next runnable process */
    for (int i = 1; i <= MAX_NPROCESS; i++) {
        int s = proc_set[(proc_curr_idx + i) % MAX_NPROCESS].status;
        if (s == PROC_READY || s == PROC_RUNNING || s == PROC_RUNNABLE) {
            next_idx = (proc_curr_idx + i) % MAX_NPROCESS;
            break;
        }
    }
#else
    int next_pid = mlfq();
    next_idx = (next_pid > 0) ? pid2idx(next_pid) : -1;
#endif

    if (next_idx == -1) FATAL("proc_yield: no runnable process");
    if (curr_status == PROC_RUNNING) proc_set_runnable(curr_pid);

    /* Switch to the next runnable process and reset timer */
    proc_curr_idx = next_idx;
    ASSERT(curr_pid > 0 && curr_pid <= MAX_NPROCESS, \
            "proc_yield: invalid pid to switch");
    earth->mmu_switch(curr_pid);
    earth->timer_reset();

    /* [lab3-ex1]
     * TODO: update "schd_attr" for the next process (new process)
     * */




    /* Call the entry point for newly created process */
    if (curr_status == PROC_READY) {
        proc_set_running(curr_pid);

        /* [lab4-ex3]
         * TODO: the kernel will switch privilege level here:
         * - if the curr_pid is a system process, set the privilege level to S-Mode
         * - if the curr_pid is a user application, set the privilege level to U-Mode
         */

        /* TODO: your code here */

        /* Prepare argc and argv */
        asm("mv a0, %0" ::"r"(APPS_ARG));
        asm("mv a1, %0" ::"r"(APPS_ARG + 4));
        /* Enter application code entry using mret */
        asm("csrw mepc, %0" ::"r"(APPS_ENTRY));
        asm("mret");
    }

    proc_set_running(curr_pid);
}

/* [lab3-ex2]
 * initialize MLFQ data structures */
void mlfq_init() {
    /* TODO: your code here*/

    FATAL("mlfq_init not implemented");



}

/* [lab3-ex2]
 * implement MLFQ scheduler
 *   - return the **pid** (NOT index) of the next runnable process.
 *   - if there is no other runnable processes and the current process is PROC_RUNNING,
 *     you should let the current process keep running.
 *   - if no process is runnable, return 0.
 *   - you should always put system processes (pid<USER_PID_START) in the highest priority queue
 *   - you should not enqueue PROC_UNUSED
 *
 * hints:
 *   - you need to check the status of the process before running it;
 *     a process might not be runnable when dequeue (for example, in a status of waiting)
 *   - some invariants you want to check (for robust MLFQ implementation):
 *     -- the current running process should not be in the queue
 *     -- all processes except the current running one are in some queue
 *     -- there are no duplicated pids between queues
 *   - use pid2idx() and idx2pid() to translate pid and index in "proc_set"
 * */
static int mlfq() {
    /* TODO: your code here */




    return 0;
}


/* process scheduling metrics */

/* [lab-ex1]
 * TODO: return turn-around time for pid
 * - the returned value is a float number (how many QUANTUM)
 */
static float tar_time(int pid) {
    return 0;
}

/* [lab-ex1]
 * TODO: return response time for pid
 * - the returned value is a float number (how many QUANTUM)
 */
static float resp_time(int pid) {
  return 0;
}

/* [lab-ex1]
 * TODO: return how many times the process pid has been scheduled
 * - the returned value is an integer
 */
static int yield_num(int pid) {
  return 0;
}

/* [lab-ex1]
 * TODO: return the actual CPU time the pid used
 * - the returned value is a float number (how many QUANTUM)
 */
static float cpu_runtime(int pid) {
  return 0;
}


/* process life-cycle functions */

/* proc_on_arrive will be called when the process is created */
void proc_on_arrive(int pid) {
    /* [lab-ex1]
     * TODO: collect pid's scheduling information
     * hint: remember to init/clear the pid's "schd_attr"
     */




#ifdef MLFQ
  static int first_time = 1;
  if (first_time) {
      mlfq_init();
      first_time = 0;
      /* return without adding proc to queue for the first time
       * because this process is sys_proc, which will directly
       * run without go via proc_yield().*/
      return;
  }

  /* [lab3-ex2]
   * TODO: add the process to MLFQ
   * Note that pid is not the curr_pid
   */





#endif
}

/* proc_on_stop() will be called when the process exits */
void proc_on_stop(int pid) {
    ASSERTX(proc_set[pid2idx(pid)].status == PROC_UNUSED); // pid must be exited

    /* [lab3-ex1]
     * TODO: collect pid's scheduling information */




    INFO("proc %d finished after %d yields, turnaround time: %.2f, response time: %.2f, cputime: %.2f",
            pid, yield_num(pid),
            tar_time(pid), resp_time(pid),
            cpu_runtime(pid));

#ifdef MLFQ
    /* [lab3-ex2]
     * TODO: remove process from queues */




#endif
}

/* [lab3-ex3]
 * let pid sleep for the given time units (i.e.., time_units of QUANTUM):
 *   - set pid's status to PROC_SLEEPING
 *   - record some information in "schd_attr" to later decide when the time is up
 *   - later (whenever scheduling is happening), check if the process should be waked up
 *   - in your scheduler (mlfq), if no process can run but there exists sleeping processes,
 *     the scheduler should not return; it should busy loop until one of the
 *     sleeping processes times up.
 *
 * hints:
 *   - you will likely modify multiple other places in your MLFQ implementation
 *   - you probably want a new sleeping queue
 *     -- if so, remember to remove this process from its original queue
 * */
void proc_on_sleep(int pid, int time_units) {
    ASSERTX(proc_set[pid2idx(pid)].status != PROC_SLEEPING);

    /* TODO: your code here */




}

