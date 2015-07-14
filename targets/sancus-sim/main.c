#include "event-loop.h"

#include <stdio.h>
#include <msp430.h>

int main(int argc, char** argv)
{
    WDTCTL = WDTPW | WDTHOLD;
    asm volatile("eint");

    puts("sancus-sim main()");

    event_loop_start();
}

