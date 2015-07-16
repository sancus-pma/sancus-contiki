#include "event-loop.h"

#include <stdio.h>
#include <msp430.h>

int main(int argc, char** argv)
{
    WDTCTL = WDTPW | WDTHOLD;
    event_loop_init();
    asm volatile("eint");

    puts("sancus-sim main()");

    event_loop_start();
}
