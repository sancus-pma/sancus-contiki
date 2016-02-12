#include "buttons.h"

#include "port1_mmio.h"
#include "buttons_driver.h"

#include <sancus_support/global_symtab.h>

static void init_symbols()
{
#define SYM(name) extern char name; add_global_symbol(#name, &name, NULL)
    SYM(__sm_buttons_driver_entry_buttons_driver_register_callback_idx);
    SYM(__sm_buttons_driver_entry);
#undef SYM
}

void buttons_init()
{
    sancus_enable(&port1_mmio);
    sancus_enable(&buttons_driver);
    buttons_driver_init();
    init_symbols();
}

void buttons_handle_events()
{
    buttons_driver_handle_events();
}
