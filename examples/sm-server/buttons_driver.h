#ifndef BUTTONS_DRIVER_H
#define BUTTONS_DRIVER_H

#include <sancus/sm_support.h>

extern struct SancusModule buttons_driver;

SM_ENTRY(buttons_driver) void buttons_driver_init(void);
SM_ENTRY(buttons_driver) void buttons_driver_handle_events(void);

#endif
