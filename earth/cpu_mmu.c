/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: memory management unit (MMU)
 *
 * updated by CS6640 23fall staff
 */

#include "egos.h"
#include <string.h>

/* Interface to allocate/free a physical page */
void* pmalloc(int clear);
void  pfree(void *paddr);
void *pageid2paddr(int pageid);

typedef struct {
    void *private_addr;
    void *exec_addr;
} page_mapping;

// number of pages of an app
#define APPS_NPAGES (APPS_SIZE/PAGE_SIZE + 2)
page_mapping app_pages[MAX_NPROCESS][APPS_NPAGES];

void* mmu_alloc() {
    return  pmalloc(1);
}

int soft_tlb_free(int pid) {
    ASSERT(pid > 0 && pid<=MAX_NPROCESS, "pid is unexpected");
    page_mapping *pages = app_pages[pid];
    for (int i = 0; i < APPS_NPAGES; i++) {
        if (pages[i].private_addr != NULL) {
            pfree(pages[i].private_addr);
            pages[i].private_addr = NULL;
            pages[i].exec_addr = NULL;
        }
    }
}

/* Software TLB Translation */
int soft_tlb_map(int pid, void *dst, void *src) {
    ASSERT(pid > 0 && pid<=MAX_NPROCESS, "pid is unexpected");
    ASSERT( ((unsigned int)src % PAGE_SIZE == 0) && \
            ((unsigned int)dst % PAGE_SIZE == 0), \
            "mapping address is not page aligned");

    page_mapping *pages = app_pages[pid];
    for (int i = 0; i < APPS_NPAGES; i++) {
        if (pages[i].private_addr == NULL) {
            pages[i].private_addr = src;
            pages[i].exec_addr = dst;
            return 0;
        }
    }

    FATAL("soft_tlb_map: mapping more pages than expected");
}

// a helper function to copy memory
void memcopy_helper(int pid, int private2exec) {
    ASSERT(pid > 0 && pid<=MAX_NPROCESS, "pid is unexpected");

    page_mapping *pages = app_pages[pid];
    for (int i = 0; i < APPS_NPAGES; i++) {
        if (pages[i].private_addr != NULL) {
            ASSERT(pages[i].exec_addr != NULL, "");

            void *src = (private2exec) ? \
                        pages[i].private_addr : pages[i].exec_addr;
            void *dst = (private2exec) ? \
                        pages[i].exec_addr : pages[i].private_addr;

            memcpy(dst, src, PAGE_SIZE);
        }
    }
}

int soft_tlb_switch(int pid) {
    static int curr_vm_pid = -1;
    if (pid == curr_vm_pid) return 0;

    /* for curr_vm_pid,
     * copy from exec memory back to the private pages*/
    if (curr_vm_pid > 0) {
        memcopy_helper(curr_vm_pid, 0 /*private2exec? no*/);
    }

    /* for pid (new running process),
     * copy from private pages to exec addresses */
    memcopy_helper(pid, 1 /*private2exec? yes*/);

    curr_vm_pid = pid;
}

void pmp_init() {
    /* [lab4-ex4]
     * TODO: set PMP memory protection */
    /* Setup PMP region [0x00000000,  0x20000000) as r/w/x */
    /* Setup PMP region [0x20400000,  0x20800000) as r/-/x */
    /* Setup PMP region [0x20800000,  0x20C00000) as r/-/- */
    /* Setup PMP region [0x80000000,  0x80400000) as r/w/- */


}

/* defined in cpu_vm.c */
void vm_init();

/* MMU Initialization */
void mmu_init() {
    /* Initialize MMU interface functions */
    earth->mmu_alloc = mmu_alloc;
    earth->mmu_free = soft_tlb_free;
    earth->mmu_map = soft_tlb_map;
    earth->mmu_switch = soft_tlb_switch;

    /* Initialize memory protection */
#ifndef VMON
    pmp_init();
#else
    /* Initialize virtual memory */
    vm_init();
#endif
}
