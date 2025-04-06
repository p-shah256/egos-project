/*
 * (C) 2025, Cornell University
 * All rights reserved.
 *
 * Description: a simple SD card driver
 *
 * Author: Yunhao Zhang
 * updated by OSI 2025 staff
 */

#include "egos.h"
#include "disk.h"

#define SPI_BASE  0x10050000UL
#define SIFIVE_SPI_CSDEF  20UL
#define SIFIVE_SPI_CSMODE 24UL
#define SIFIVE_SPI_TXDATA 72UL
#define SIFIVE_SPI_RXDATA 76UL

/* SPI and SD card helper functions */

/* [lab6-ex1] read the following function
 *
 * Exchange a byte between SPI's master and slave
 * "exchange" means sending a byte and then receiving a byte
 */
static char spi_exchange(char byte) {
    m_uint32 rxdata;
    while (REGW(SPI_BASE, SIFIVE_SPI_TXDATA) & (1 << 31));
    REGW(SPI_BASE, SIFIVE_SPI_TXDATA) = byte;
    while ((rxdata = REGW(SPI_BASE, SIFIVE_SPI_RXDATA)) & (1 << 31));

    return (char)(rxdata & 0xFF);
}

/* [lab6-ex1] read the following function
 *
 * Send SD card commands through the SPI bus
 */
static char sd_exec_cmd(char* cmd) {
    for (m_uint32 i = 0; i < 6; i++) spi_exchange(cmd[i]);

    for (m_uint32 reply, i = 0; i < 8000; i++)
        if ((reply = spi_exchange(0xFF)) != 0xFF) return reply;

    return 0xFF;
}

/* Read/write SD card blocks */

/* Read a single block from the given block number */
static void sd_single_read(m_uint32 block_no, char* dst) {
    /* QEMU uses the SD2 standard (offset is byte offset) */
    int offset = block_no * BLOCK_SIZE;

    /* Wait until SD card is not busy */
    while (spi_exchange(0xFF) != 0xFF);

    /* Send read request with cmd17 */
    char reply;

    /* [lab6-ex1]
     * Construct the command frame for a single block read (CMD17).
     * Refer to the "(a) SPI command" section of the instructions:
     * - the command frame consists of 6 bytes.
     * - the first two bits are "01".
     * - followed by a 6-bit command index.
     * - followed by the 4-byte reading address (i.e., the "offset" variable).
     * - finally, a 1-byte CRC (ignored and replaced with 0xFF).
     * - remove the "FATAL" line below after finishing
     *
     * Hints:
     * - setting the 4-byte "offset" in the frame is tricky.
     * - consider the CPU's little-endian format and the SPI byte transmission order.
     */
    char cmd17[] = {
        /* TODO: your code here */
        0x00, 0x00, 0x00, 0x00, 0x00, 0xFF
    };
    FATAL("sd_single_read(): cmd17 frame is not implemented.");

    if (reply = sd_exec_cmd(cmd17))
        FATAL("SD card replies cmd17 with status 0x%x", reply);

    /* Wait for the data packet and ignore the 2-byte checksum (CRC) */
    while (spi_exchange(0xFF) != 0xFE);
    for (m_uint32 i = 0; i < BLOCK_SIZE; i++) dst[i] = spi_exchange(0xFF);
    spi_exchange(0xFF); // CRC[0]
    spi_exchange(0xFF); // CRC[1]
}

/* Write a single block to the given block number */
static void sd_single_write(m_uint32 block_no, char* src) {
    /* QEMU uses the SD2 standard (offset is byte offset) */
    int offset = block_no * BLOCK_SIZE;

    /* Wait until SD card is not busy */
    while (spi_exchange(0xFF) != 0xFF);


    /* [lab6-ex2]
     * Complete writing a block to SD card.
     * Here are what you should do:
     * 1. send write request with cmd24
     * 2. transfer 1-byte buffer before writing block
     * 3. send data packet: token + data block + dummy 2-byte checksum
     *    -- token: this is the data token of the data packet
     *    -- data block: a BLOCK_SIZE bytes of data from the "src" pointer
     *    -- checksum: you can use 0xFFFF
     */
    /* TODO: your code here */
    FATAL("sd_single_write is incomplete");


    /* Wait for SD card ack of data packet */
    char reply;
    while ((reply = spi_exchange(0xFF)) == 0xFF);
    if ((reply & 0x1F) != 0x05)
        FATAL("SD card write ack with status 0x%x", reply);
}

/* [lab6-ex3]
 * Implement multi block read using SD card cmd18 and cmd12
 */
static void sd_multi_read(m_uint32 block_no, char* dst, int nblocks) {
    /* TODO: your code here */

    FATAL("sd_multi_read is not implemented");

}


/* [lab6-ex4]
 * Implement multi block write using SD card cmd25 and "stop trans token"
 */
static void sd_multi_write(m_uint32 block_no, char* src, int nblocks) {
    /* TODO: your code here */

    FATAL("sd_multi_write is not implemented");

}

/* SD card read/write interfaces */

void sd_read(m_uint32 block_no, m_uint32 nblocks, char* dst) {
    if (nblocks == 1) {
        sd_single_read(block_no, dst);
    } else if (nblocks > 1) {
        sd_multi_read(block_no, dst, nblocks);
    } else {
        FATAL("invalid nblocks = %d", nblocks);
    }
}

void sd_write(m_uint32 block_no, m_uint32 nblocks, char* src) {
    if (nblocks == 1) {
        sd_single_write(block_no, src);
    } else if (nblocks > 1) {
        sd_multi_write(block_no, src, nblocks);
    } else {
        FATAL("invalid nblocks = %d", nblocks);
    }
}


/* Initialize the SD card during bootup */

static char sd_exec_acmd(char* cmd) {
    char cmd55[] = {0x77, 0x00, 0x00, 0x00, 0x00, 0xFF};
    while (spi_exchange(0xFF) != 0xFF);
    sd_exec_cmd(cmd55);

    while (spi_exchange(0xFF) != 0xFF);
    return sd_exec_cmd(cmd);
}

int sd_init() {
    /* Configure the SPI controller */
    INFO("Set the CS pin to HIGH and toggle clock");

    REGW(SPI_BASE, SIFIVE_SPI_CSMODE) = 2;
    for (m_uint32 i = 0; i < 1000; i++) spi_exchange(0xFF);
    REGW(SPI_BASE, SIFIVE_SPI_CSDEF) = 1;

    INFO("Set the CS pin to LOW and send cmd0 to SD card");
    char reply, cmd0[] = {0x40, 0x00, 0x00, 0x00, 0x00, 0x95};
    if ((reply = sd_exec_cmd(cmd0)) == 0xFF) return -1;
    while (reply != 0x01) reply = spi_exchange(0xFF);
    while (spi_exchange(0xFF) != 0xFF);

    INFO("Check SD card type and voltage with cmd8");
    char cmd8[] = {0x48, 0x00, 0x00, 0x01, 0xAA, 0x87};
    reply       = sd_exec_cmd(cmd8);

    if (reply & 0x04) {
        /* Illegal command */
        FATAL("Only SD2/SDHC/SDXC cards are supported");
    } else {
        /* Only need the last byte of the r7 response */
        m_uint32 payload;
        for (m_uint32 i = 0; i < 4; i++)
            ((char*)&payload)[3 - i] = spi_exchange(0xFF);
        INFO("SD card replies cmd8 with status %d", reply);

        if ((payload & 0xFFF) != 0x1AA) FATAL("Fail to check SD card type");
    }
    while (spi_exchange(0xFF) != 0xFF);

    char acmd41[] = {0x69, 0x40, 0x00, 0x00, 0x00, 0xFF};
    while (sd_exec_acmd(acmd41));
    while (spi_exchange(0xFF) != 0xFF);

    return 0;
}

