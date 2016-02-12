#include "buttons_process.h"
#include "buttons.h"

#include <stdio.h>

#define LOG(...) printf("Buttons: " __VA_ARGS__)

PROCESS(buttons_process, "Reactive process");

PROCESS_THREAD(buttons_process, ev, data)
{
    PROCESS_BEGIN();

    LOG("started\n");

    buttons_init();

    while (1)
    {
        PROCESS_WAIT_EVENT();
        buttons_handle_events();
    }

    PROCESS_END();
}

void buttons_process_poll(void)
{
    process_poll(&buttons_process);
}
