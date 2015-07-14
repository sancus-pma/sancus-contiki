#include "event-loop.h"

#include <stdio.h>
#include <msp430.h>

#include <sancus_support/uart.h>

int main(int argc, char** argv)
{
    WDTCTL = WDTPW | WDTHOLD;
    uart_init();
    asm volatile("eint");

    puts("sancus-sim main()");

    event_loop_start();
}

