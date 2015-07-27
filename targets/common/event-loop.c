#include "event-loop.h"

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

    while (1)
    {
        int nEvents;
        do
        {
            etimer_request_poll();
            nEvents = process_run();
        }
        while (nEvents > 0);
    }
}
