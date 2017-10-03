#include "event-loop.h"
#include "networking.h"

#include <msp430.h>
#include <stdio.h>

#include <sancus_support/uart.h>
#include <sancus_support/tsc.h>

void wait_cycles(tsc_t cycles)
{
    tsc_t start = tsc_read();

    while (tsc_read() - start < cycles) {}
}

int main(int argc, char** argv)
{
    WDTCTL = WDTPW | WDTHOLD;
    wait_cycles(20000000);
    uart_init();
    event_loop_init();
    networking_init();
    asm volatile("eint");

    puts("sancus-net main()");

    event_loop_start();
}
