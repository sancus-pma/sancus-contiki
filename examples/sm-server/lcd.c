#include "lcd.h"

#include "lcd_mmio.h"
#include "lcd_driver.h"

#include <sancus_support/global_symtab.h>

static void init_symbols()
{
#define SYM(name) extern char name; add_global_symbol(#name, &name, NULL)
    SYM(__sm_lcd_driver_entry_lcd_driver_write_idx);
    SYM(__sm_lcd_driver_entry_lcd_driver_acquire_idx);
    SYM(__sm_lcd_driver_entry);
#undef SYM
}

void lcd_init()
{
    sancus_enable(&lcd_mmio);
    sancus_enable(&lcd_driver);
    lcd_driver_init();
    init_symbols();
}

void lcd_clear()
{
    lcd_driver_clear();
}

void lcd_write(const char* text)
{
    lcd_driver_write(text);
}
