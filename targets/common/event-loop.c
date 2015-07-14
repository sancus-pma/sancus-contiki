#include "event-loop.h"

#include "contiki.h"

void event_loop_start()
{
    process_init();
    autostart_start(autostart_processes);

    while (1)
    {
        int nEvents;
        do
        {
            nEvents = process_run();
        }
        while (nEvents > 0);
    }
}

