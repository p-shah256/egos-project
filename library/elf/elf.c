/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: load an ELF-format executable file into memory
 * Only use the program header instead of the multiple section headers.
 */

#include "egos.h"
#include "elf.h"
#include "disk.h"
#include "servers.h"

#include <string.h>

static void load_grass(elf_reader reader,
                       struct elf32_program_header* pheader) {
    INFO("Grass kernel file size: 0x%.8x bytes", pheader->p_filesz);
    INFO("Grass kernel memory size: 0x%.8x bytes", pheader->p_memsz);

    char* entry = (char*)GRASS_ENTRY;
    int block_offset = pheader->p_offset / BLOCK_SIZE;
    for (int off = 0; off < pheader->p_filesz; off += BLOCK_SIZE)
        reader(block_offset++, entry + off);

    memset(entry + pheader->p_filesz, 0, GRASS_SIZE - pheader->p_filesz);
}

static void load_app(int pid, elf_reader reader,
                     int argc, void** argv,
                     struct elf32_program_header* pheader) {

    /* Debug printing during bootup */
    if (pid < GPID_USER_START) {
        INFO("App file size: 0x%.8x bytes", pheader->p_filesz);
        INFO("App memory size: 0x%.8x bytes", pheader->p_memsz);
    }

    /* load app segment into planned memory:
     *  (1) allocate memory for app's code+data
     *      [APPS_ENTRY, APPS_ENTRY+APPS_SZIE]
     *  (2) copy code to the allocated memory
     *  (3) allocate stack and arg pages
     * */

    // (1) allocate memory
    int app_npages = APPS_SIZE / PAGE_SIZE;
    void *paddrs[app_npages];

    for (int i = 0; i < app_npages; i++) {
        paddrs[i] = earth->mmu_alloc();
        memset((char*)paddrs[i], 0, PAGE_SIZE);

        // map allocated page to app's corresponding address
        earth->mmu_map(pid, (void *) (APPS_ENTRY + i * PAGE_SIZE), paddrs[i]);
    }

    // (2) copy app from elf to the allocated memory
    int page_idx = -1;
    int block_offset = pheader->p_offset / BLOCK_SIZE;
    for (int off = 0; off < pheader->p_filesz; off += BLOCK_SIZE) {
        if (off % PAGE_SIZE == 0) {
            page_idx++;  // first time: page_idx == 0
            ASSERT( ((page_idx>=0) && (page_idx<app_npages)), "app is larger than APPS_SIZE");
        }
        reader(block_offset++, (char*)paddrs[page_idx] + (off % PAGE_SIZE));
    }

    // clean the unfilled memory on the last page
    int last_page_filled = pheader->p_filesz % PAGE_SIZE;
    int last_page_nzeros = PAGE_SIZE - last_page_filled;
    if (last_page_filled) {
        memset((char*)paddrs[page_idx] + last_page_filled, 0, last_page_nzeros);
    }


    /* (3) Setup two pages for app args, syscall args, and app stack:
     *       [APPS_ARG, APPS_STACK_TOP)
     * */
    void *paddr = earth->mmu_alloc();
    earth->mmu_map(pid, (void *)APPS_ARG, paddr);

    int* argc_addr = (int *)paddr;
    int* argv_addr = argc_addr + 1;
    int* args_addr = argv_addr + CMD_NARGS;

    *argc_addr = argc;
    if (argv) memcpy(args_addr, argv, argc * CMD_ARG_LEN);
    for (int i = 0; i < argc; i++)
        argv_addr[i] = APPS_ARG + 4 + 4 * CMD_NARGS + i * CMD_ARG_LEN;

    paddr = earth->mmu_alloc();
    earth->mmu_map(pid, (void*)(APPS_ARG + PAGE_SIZE), paddr);
}

void elf_load(int pid, elf_reader reader, int argc, void** argv) {
    char buf[BLOCK_SIZE];
    reader(0, buf);

    struct elf32_header *header = (void*) buf;
    ASSERT(header->e_phnum == 1, "has more than one program header");

    struct elf32_program_header *pheader = (void*)(buf + header->e_phoff);

    if (pheader->p_vaddr == GRASS_ENTRY)
        load_grass(reader, pheader);
    else if (pheader->p_vaddr == APPS_ENTRY)
        load_app(pid, reader, argc, argv, pheader);
    else
        FATAL("elf_load: ELF gives invalid p_vaddr: 0x%.8x", pheader->p_vaddr);
}


