/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: Initialize the bss and data segments;
 * Initialize dev_tty, dev_disk, cpu_intr and cpu_mmu;
 * Load the grass layer binary from disk and run it.
 *
 * updated by CS6640 23fall staff
 */

#include "elf.h"
#include "disk.h"
#include "egos.h"
#include <string.h>

void tty_init();
void disk_init();
void intr_init();
void timer_init();
void mmu_init();

struct grass *grass = (void*)GRASS_STRUCT_BASE;
struct earth *earth = (void*)EARTH_STRUCT_BASE;
extern char bss_start, bss_end, data_rom, data_start, data_end;

static void earth_init() {
    tty_init();
    CRITICAL("------------- Booting -------------");
    SUCCESS("Finished initializing the tty device");

    intr_init();
    SUCCESS("Finished initializing the CPU interrupts");

    timer_init();
    SUCCESS("Finished initializing the timer function");

    disk_init();
    SUCCESS("Finished initializing the disk device");

    mmu_init();
    SUCCESS("Finished initializing the CPU memory management unit");
}

static int grass_read(int block_no, char* dst) {
    return earth->disk_read(GRASS_EXEC_START + block_no, 1, dst);
}

int main() {
    m_uint32 core_id;
    asm("csrr %0, mhartid" : "=r"(core_id));

    /* Disable core#0 on QEMU because it is an E31 core without S-mode */
    /* See https://www.qemu.org/docs/master/system/riscv/sifive_u.html */
    if (core_id != 0) {
        while (1);
    }

    /* Initialize the earth layer */
    earth_init();

    /* Load and enter the grass layer */
    elf_load(0, grass_read, 0, 0);

    /* Enable machine-mode interrupt before entering supervisor mode */
    earth->timer_reset();
    earth->intr_enable();

    void (*grass_entry)() = (void*)GRASS_ENTRY;
    grass_entry();
}
