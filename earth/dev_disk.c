/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: a simple disk device driver
 *
 * updated by CS6640 23fall staff
 */

#include "egos.h"
#include "disk.h"
#include "bus_gpio.c"
#include <string.h>


int sd_init();
void sd_read(int block_no, int nblocks, char* dst);
void sd_write(int block_no, int nblocks, char* src);
void sd_test();

static enum disk_type { FLASH_ROM, SD_CARD } type;

int disk_read(int block_no, int nblocks, char* dst) {
    if (type == FLASH_ROM) {
        char* src = (char*)0x20800000 + block_no * BLOCK_SIZE;
        memcpy(dst, src, nblocks * BLOCK_SIZE);
    } else if (type == SD_CARD) {
        sd_read(block_no, nblocks, dst);
    } else {
        ASSERT(0, "disk type is unknown");
    }
    return 0;
}

int disk_write(int block_no, int nblocks, char* src) {
    if (type == FLASH_ROM) {
        FATAL("Attempt to write the on-board ROM");
    } else if (type == SD_CARD) {
        sd_write(block_no, nblocks, src);
    } else {
        ASSERT(0, "write to disk type unknown");
    }
    return 0;
}

void disk_init() {
    earth->disk_read = disk_read;
    earth->disk_write = disk_write;

#ifndef SDCARD
    type = FLASH_ROM;
#else
    type = SD_CARD;
    if (sd_init() != 0) {
        FATAL("SD card initialization failed");
    }
    //sd_test();
#endif

    return;
}


#define SD_FREE_START 0 //(2*1024*1024 - 4096)

void sd_test() {
    CRITICAL("Start SD card testing");
    int buf[1024];    // 4KB
    char buf2[4096];  // 4KB
    m_uint32 block_no = SD_FREE_START/BLOCK_SIZE;

    // test 1: test write single block
    for (int i=0; i<BLOCK_SIZE/sizeof(int); i++) { // init buf
        buf[i] = i;
    }
    sd_write(block_no, 1, (char*)buf);  // write one block
    sd_read(block_no, 1, (char*)buf2);  // read the block back
    if (memcmp(buf, buf2, BLOCK_SIZE) == 0) { // check results
        INFO("single-write test passed!");
    } else {
        FATAL("single-write test failed!");
    }

    // test 2: test multi-block read
    m_uint32 nblocks = 4096/BLOCK_SIZE;  // 4KB/512B -> 8
    for (int i=0; i<1024; i++) { // init buf
        buf[i] = i*i;
    }
    for (int i=0; i<nblocks; i++) {
        sd_write(block_no+i, 1, &((char*)buf)[i*BLOCK_SIZE]); // write buf to SD card
    }
    sd_read(block_no, nblocks, buf2);
    if (memcmp(buf, buf2, 4096) == 0) {
        INFO("multi-read test passed!");
    } else {
        FATAL("multi-read test failed!");
    }

    // test 3: test multi-block write
    for (int i=0; i<1024; i++) { // init buf
        buf[i] = i*i*i;
    }
    sd_write(block_no, nblocks, (char*)buf); // write buf to SD card
    sd_read(block_no, nblocks, buf2);
    if (memcmp(buf, buf2, 4096) == 0) {
        INFO("multi-write test passed!");
    } else {
        FATAL("multi-write test failed!");
    }

    /* [lab6-ex5]
     * write more test cases to test your code
     */

    /* TODO: your code here */

}
