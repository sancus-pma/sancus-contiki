#include "event-loop.h"

#include <stdio.h>
#include <sancus_support/sm_io.h>

int main(int argc, char** argv)
{
    msp430_io_init();
    event_loop_init();
    asm volatile("eint");

    puts("sancus-bare main()");

    event_loop_start();
}
