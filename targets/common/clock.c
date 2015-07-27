#include "sys/clock.h"

#include <sancus_support/tsc.h>

#include <stdio.h>

void clock_init()
{
}

clock_time_t clock_time()
{
    return tsc_read();
}
