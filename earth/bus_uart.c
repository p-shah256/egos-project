/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: definitions for UART0 in FE310
 * see chapter18 of the SiFive FE310-G002 Manual
 */

#include "egos.h"
#include "bus_gpio.c"

#define UART_BASE     0x10010000UL

#define SIFIVE_UART_TXDATA 0UL
#define SIFIVE_UART_RXDATA 4UL

int uart_getc(int* c) {
    int ch = REGW(UART_BASE, SIFIVE_UART_RXDATA);
    return *c = (ch & (1 << 31))? -1 : (ch & 0xFF);
}

void uart_putc(char c) {
        while ((REGW(UART_BASE, SIFIVE_UART_TXDATA) & (1 << 31)));
        REGW(UART_BASE, SIFIVE_UART_TXDATA) = c;
}
