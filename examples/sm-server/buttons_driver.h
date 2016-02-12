#ifndef BUTTONS_DRIVER_H
#define BUTTONS_DRIVER_H

#include <sancus/sm_support.h>

typedef enum
{
    Button1 = 0,
    Button2 = 1
} Button;

extern struct SancusModule buttons_driver;

SM_ENTRY(buttons_driver) void buttons_driver_init(void);
SM_ENTRY(buttons_driver) void buttons_driver_handle_events(void);
SM_ENTRY(buttons_driver) int  buttons_driver_register_callback(
                                    Button button, void* entry, unsigned index);

#endif
