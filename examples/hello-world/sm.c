#include <stdio.h>

#include <sancus/sm_support.h>

void SM_ENTRY("sm") sm_entry(void)
{
    puts("Hello from an SM");
}

DECLARE_SM(sm, 1234);

void call_sm()
{
    puts("Enabling SM");
    sancus_enable(&sm);
    sm_entry();
}

