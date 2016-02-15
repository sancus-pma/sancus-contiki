#include "lcd_driver.h"

#include "lcd_mmio.h"

SM_DATA(lcd_driver) sm_id current_sm;

SM_FUNC(lcd_driver) int access_allowed()
{
    return current_sm == 0 || current_sm == sancus_get_caller_id();
}

SM_ENTRY(lcd_driver) void lcd_driver_init()
{
    if (!access_allowed())
        return;

    lcd_mmio_init();
    lcd_driver_clear();
}

SM_ENTRY(lcd_driver) void lcd_driver_acquire()
{
    if (current_sm != 0)
        return;

    current_sm = sancus_get_caller_id();
}

SM_ENTRY(lcd_driver) void lcd_driver_release()
{
    if (!access_allowed())
        return;

    current_sm = 0;
}

SM_ENTRY(lcd_driver) void lcd_driver_clear()
{
    lcd_driver_write("\x1b[j");
}

SM_ENTRY(lcd_driver) void lcd_driver_write(const char* text)
{
    if (!access_allowed())
        return;

    while (*text != '\0')
        lcd_mmio_write_byte(*text++);
}

DECLARE_SM(lcd_driver, 0x1234);
