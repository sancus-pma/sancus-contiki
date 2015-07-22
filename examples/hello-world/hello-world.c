#include "contiki.h"

#include <stdio.h>

#include "sm.h"

PROCESS(hello_world_process, "Hello world process");

AUTOSTART_PROCESSES(&hello_world_process);

PROCESS_THREAD(hello_world_process, ev, data)
{
    PROCESS_BEGIN();

    printf("Hello, world\n");
    call_sm();

    PROCESS_END();
}

