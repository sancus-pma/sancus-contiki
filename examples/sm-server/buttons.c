#include "buttons.h"

#include "port1_mmio.h"
#include "buttons_driver.h"

void buttons_init()
{
    sancus_enable(&port1_mmio);
    sancus_enable(&buttons_driver);
    buttons_driver_init();
}

void buttons_handle_events()
{
    buttons_driver_handle_events();
}
