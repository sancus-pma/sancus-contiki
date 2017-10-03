#include "event-loop.h"

#include <sancus_support/sm_io.h>
#include "contiki.h"

void event_loop_init()
{
    clock_init();
    process_init();
    process_start(&etimer_process, NULL);
}

void event_loop_start()
{
    autostart_start(autostart_processes);

    int nEvents;
    do
    {
        etimer_request_poll();
        nEvents = process_run();
    }
    while (nEvents > 0);

    EXIT();
}
