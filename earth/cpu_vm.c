/* Description: Virtual Memory Management*/

#include "egos.h"
#include "string.h"

/* Page Table Translation
 *
 * The code below creates an identity mapping using RISC-V Sv32;
 * Read section4.3 of RISC-V privileged spec manual (riscv-privileged-v1.10.pdf)
 */

#define FLAG_VALID_RWX 0xF
#define FLAG_NEXT_LEVEL 0x1

/* Interface to allocate/free a physical page */
void* pmalloc(int clear_page);
void  pfree(void *paddr);


#define MAX_ROOT_PAGE_TABLES MAX_NPROCESS
#define USER_PID_START 5  // the first user app pid

/* a mapping from pid to page table root */
static m_uint32* pid_to_pagetable_base[MAX_ROOT_PAGE_TABLES];

void fence() {
    asm("sfence.vma zero,zero");
}


/* [lab5-ex1]:
 * this function walks the page table:
 *   it translates virtual address "va" via page table "root",
 *   and returns the **address** of the PTE pointing to "va".
 * if alloc is non-zero,
 *   allocate the pages when necessary.
 * if alloc is zero,
 *   FATAL when cannot find the page.
 *
 * hints:
 *  - the return value is a PTE pointer.
 *    With the pointer, it is easy to change the permissions on PTE.
 *  - use pmalloc() to allocate pages
 */
m_uint32* walk(m_uint32* root, void* va, int alloc) {
    /* TODO: your code here */
    m_uint32 *l2_pte_ptr = NULL;





    return l2_pte_ptr;
}


/* [lab5-ex1]
 * establish the mapping between the "va" to the "pa" for process "pid"
 */
int page_table_map(int pid, void *va, void *pa) {
    ASSERT(pid >= 0 && pid <= MAX_NPROCESS, "pid is unexpected");
    ASSERT( ((m_uint32)va % PAGE_SIZE == 0) &&
            ((m_uint32)pa % PAGE_SIZE == 0), "page is not aligned");


    m_uint32 *root = pid_to_pagetable_base[pid];

    /* Implement va-pa mapping:
     * (1) if the page table for pid does not exist, build the page table.
     *     (a) if the process is a system process (pid < USER_PID_START)
     *       you need to map the well-known memory to system process address space.
     *       Here are the well-known memory regions:
     *           | start address | #pages| explanation
     *           +---------------+-------+------------------
     *           |  0x02000000   | 16    | CLINT
     *           |  0x08000000   | 512   | earth data, grass code+data
     *           |  0x10013000   | 1     | UART0
     *           |  0x20400000   | 256   | earth code
     *           |  0x20800000   | 1024  | disk data
     *           |  0x80000000   | 1024  | DTIM memory
     *       hint: you should use "setup_identity_region()"
     *     (b) if the process is a user process (pid >= USER_PID_START)
     *       you need to map the following well-known memory to the user address space.
     *           | start address | #pages| explanation
     *           +---------------+-------+------------------
     *           |  0x08000000   | 512   | earth data, grass code+data
     *           |  0x10013000   | 1     | UART0
     *           |  0x20400000   | 256   | earth code
     *           |  0x80002000   | 2     | grass interface
     *           |               |       | + earth/grass stack
     *           |               |       | + earth interface
     *
     * (2) if the page tables exists,
     *   use walk() to get the PTE pointer,
     *   and update PTE accordingly.
     *   note:
     *   system processes run in supervisor mode; others run in user mode.
     *   You should update their PTE permission bits accordingly.
     */

    /* TODO: your code here */
    FATAL("page_table_map is not implemented.");






    /* wait flushing TLB entries */
   fence();
}


/* [lab5-ex2]
 * switching address space to process "pid"
 * hints:
 * - you will use "asm" to manipulate CSR "satp"
 */
int page_table_switch(int pid) {
    ASSERT(pid >= 0 && pid <= MAX_NPROCESS, "pid is unexpected");
    /* ensure all updates to page table are settled */
    fence();

    /* TODO: your code here */
    FATAL("page_table_switch is not implemented.");


    /* wait flushing TLB entries */
    fence();
}



/* [lab5-ex3]
 * this function translates the virtual address "va" to its physical address,
 * and returns the physical address.
 */
void *page_table_translate(int pid, void *va) {

    /* TODO: your code here */
    FATAL("page_table_translate is not implemented.");

    return NULL;
}


/* [lab5-ex4]
 * free page table and all relevant pages
 */
int page_table_free(int pid) {
    ASSERT(pid >= 0 && pid <= MAX_NPROCESS, "pid is unexpected");
    /* To free the page table:
     * (1) free all pages pointed by the page table root
     *     by calling pfree()
     *     (a) loop L1 pte
     *       (b) loop L2 pte
     *         (c) free data page
     *       (b) free L2 page table page
     *     (a) free L1 page table page (pointed by root)
     * (2) set pid_to_pagetable_base[pid] to 0
     *
     * Note:
     * - use pfree() to free pages
     * - take care of the well-known pages (don't free the pages that are not
     *   allocated by your code)
     */

    FATAL("page_table_free is not implemented.");
    /* TODO: your code here */




    /* wait flushing TLB entries */
    fence();
}


/* set up virtual memory mapping for process pid,
 * by mapping
 *      VA [addr, addr + npages*PAGE_SIZE)
 * to
 *      PA [addr, addr + npages*PAGE_SIZE)
 * */
void setup_identity_region(int pid, m_uint32 addr, int npages) {
    ASSERT(npages <= 1024, "npages is larger than 1024");
    ASSERT((addr & 0xFFF) == 0, "addr is not 4K aligned");

    m_uint32* root = pid_to_pagetable_base[pid];
    ASSERT(root != NULL, "pagetable root is NULL");

    /* Setup the entry in the root page table */
    m_uint32* l2_pa;
    int vpn1 = addr >> 22;
    if (root[vpn1] & FLAG_NEXT_LEVEL) { // l2 PT page exists
        l2_pa = (m_uint32*)  ((root[vpn1] >> 10) << 12);
    } else {
        /* Allocate the L2 page table page */
        l2_pa = pmalloc(1);
        m_uint32 ppn = ((m_uint32)l2_pa >> 12);
        root[vpn1] =  (ppn << 10) | FLAG_NEXT_LEVEL;
    }

    /* Setup the PTE in the l2 page table page */
    int vpn0 = (addr >> 12) & 0x3FF;
    for (int i = 0; i < npages; i++) {
        ASSERT(l2_pa[vpn0 + i] == 0, "non-empty l2 PTE");
        /* [lab5-ex3]
         * TODO: for user processes, set PTE_U */
        l2_pa[vpn0 + i] = ((addr + i * PAGE_SIZE) >> 2) | FLAG_VALID_RWX;
    }
}

void kernel_identity_mapping() {
    /* Allocate kernel's page table.
     * It maps kernel's VA to the identical PA. */
    void *kernel_root = pmalloc(1);
    pid_to_pagetable_base[0] = (m_uint32*)kernel_root;

    /* Allocate the leaf page tables */
    setup_identity_region(0, 0x02000000, 16);   /* CLINT */
    setup_identity_region(0, 0x10013000, 1);    /* UART0 */
    setup_identity_region(0, 0x10024000, 1);    /* SD card */
    setup_identity_region(0, 0x20400000, 1024); /* boot ROM */
    setup_identity_region(0, 0x20800000, 1024); /* disk image */
    setup_identity_region(0, 0x80000000, 1024); /* DTIM memory */

    for (int i = 0; i < 8; i++) {                 /* ITIM memory is 32MB on QEMU */
        setup_identity_region(0, 0x08000000 + i * 0x00400000, 1024);
    }
    fence();
}

int vm_init() {
    /* Setup identity mapping for kernel */
    kernel_identity_mapping();

    void *kernel_root = pid_to_pagetable_base[0];
    /* switch to kernel's address space */
    asm("csrw satp, %0" ::"r"(((m_uint32)kernel_root >> 12) | (1 << 31)));

    earth->mmu_map = page_table_map;
    earth->mmu_free = page_table_free;
    earth->mmu_switch = page_table_switch;
    earth->mmu_translate = page_table_translate;
}
