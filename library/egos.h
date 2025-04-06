#pragma once

struct earth {
    /* CPU interface */
    int (*intr_enable)();
    int (*trap_handler_register)(void (*handler)(unsigned int));

    void (*timer_reset)();
    unsigned long long (*gettime)();

    void* (*mmu_alloc)();
    int   (*mmu_free)(int pid);
    int   (*mmu_map)(int pid, void *src, void *dst);
    int   (*mmu_switch)(int pid);
    void* (*mmu_translate)(int pid, void *va);

    /* Devices interface */
    int (*disk_read)(int block_no, int nblocks, char* dst);
    int (*disk_write)(int block_no, int nblocks, char* src);

    int (*tty_intr)();
    int (*tty_read)(char* buf, int len);
    int (*tty_write)(char* buf, int len);

    int (*tty_printf)(const char *format, ...);
    int (*tty_info)(const char *format, ...);
    int (*tty_fatal)(const char *format, ...);
    int (*tty_success)(const char *format, ...);
    int (*tty_critical)(const char *format, ...);
};

struct grass {
    /* Shell environment variables */
    int workdir_ino;
    char workdir[128];

    /* Process control interface */
    int  (*proc_alloc)();
    void (*proc_sleep)(int pid, int time_units);
    void (*proc_free)(int pid);
    void (*proc_set_ready)(int pid);

    /* System call interface */
    void (*sys_exit)(int status);
    void (*sys_yield)();
    void (*sys_sleep)(int time_units);
    int  (*sys_send)(int pid, char* msg, int size);
    int  (*sys_recv)(int* pid, char* buf, int size);
};

extern struct earth *earth;
extern struct grass *grass;

/* Memory layout */
#define PAGE_SIZE          4096        /* 4KB */

#ifdef SIFIVE_U

  #define GRASS_SIZE         0x00100000  /* 1MB */
  #define APPS_SIZE          0x00004000  /* 16KB */

  /* Memory regions */
  #define FREE_MEM_END      0x81000000 /* 16MB memory in total     */
  #define FREE_MEM_START    0x80800000 /* 8MB⬆️ free for mmu_alloc */
  #define APPS_STACK_TOP    0x80800000 /* 2MB⬇️ app stack          */
  #define SYSCALL_ARG       0x80601000 /* struct syscall           */
  #define APPS_ARG          0x80600000 /* main() argc/argv         */
  #define APPS_ENTRY        0x80400000 /* 2MB⬆️ app code and data  */

  #define GRASS_STACK_TOP   0x80400000 /* 2MB⬇️ egos stack         */
  #define GRASS_STRUCT_BASE 0x80201000 /* struct grass             */
  #define EARTH_STRUCT_BASE 0x80200000 /* struct earth             */
                                       /* 1MB    grass code+data  */
  #define GRASS_ENTRY       0x80100000 /* grass code start        */
                                       /* 1MB    earth data       */
                                       /* earth code start        */
  #define RAM_START         0x80000000 /* 2MB⬆️ egos code and data */

#else

  /* DTIM (see FE310-QEMU Memory Map) */
  #define FREE_MEM_END       0x80400000  /*        end of DTIM           */
  #define FREE_MEM_START     0x80004000  /*        free memory           */
                                         /* 128B   earth interface       */
  #define GRASS_STACK_TOP    0x80003f80  /* 8KB    earth/grass stack     */
                                         /*        grass interface       */
  #define APPS_STACK_TOP     0x80002000  /* 6KB    app stack             */
  #define SYSCALL_ARG        0x80000400  /* 1KB    system call args      */
  #define APPS_ARG           0x80000000  /* 1KB    app main() argc, argv */


  /* ITIM (see FE310-QEMU Memory Map) */
  #define GRASS_SIZE         0x00100000  /* 1MB */
  #define APPS_SIZE          0x00004000  /* 16KB */

  #define ITIM_END           0x0a000000  /*        end of ITIM           */
  #define APPS_ENTRY         0x08200000  /* 1MB    app code+data         */
  #define GRASS_ENTRY        0x08100000  /* 1MB    grass code+data       */
                                         /* 1MB    earth data            */
                                         /* earth code is in QSPI flash  */
  #define ITIM_START         0x08000000

#endif

#ifndef LIBC_STDIO
/* Only earth/dev_tty.c uses LIBC_STDIO and does not need these macros */
#define printf             earth->tty_printf
#define INFO               earth->tty_info
#define FATAL              earth->tty_fatal
#define SUCCESS            earth->tty_success
#define CRITICAL           earth->tty_critical
#endif

#define ASSERT(cond, msg)  \
    do { if(!(cond)) {earth->tty_fatal("[ASSERT FAILED]" msg "[file:%s, line:%d]", __FILE__, __LINE__);} } while(0)
#define ASSERTX(cond) ASSERT(cond, "")

/* Memory-mapped I/O register access macros */
#define ACCESS(x) (*(__typeof__(*x) volatile *)(x))
#define REGW(base, offset) (ACCESS((unsigned int*)(base + offset)))
#define REGB(base, offset) (ACCESS((unsigned char*)(base + offset)))

/* process */
#define MAX_NPROCESS  16

/* translating between pid and idx (in proc_set) */
#define pid2idx(pid)  ((pid>=1 && pid<=MAX_NPROCESS) ? (pid-1) \
                            : FATAL("pid2idx: invalid pid"))
#define idx2pid(idx)  ((idx>=0 && idx<MAX_NPROCESS) ? (idx+1) \
                            : FATAL("idx2pid: invalid idx"))

/* data types */
typedef unsigned int m_uint32;
typedef unsigned long long m_uint64;

/* scheduler */
#define QUANTUM  0x100000
