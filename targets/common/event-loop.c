#include "event-loop.h"

#include "contiki.h"

void event_loop_init()
{
    process_init();
}

void event_loop_start()
{
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
