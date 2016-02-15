#include "lcd.h"

#include "lcd_mmio.h"
#include "lcd_driver.h"

void lcd_init()
{
    sancus_enable(&lcd_mmio);
    sancus_enable(&lcd_driver);
    lcd_driver_init();
}

void lcd_clear()
{
    lcd_driver_clear();
}

void lcd_write(const char* text)
{
    lcd_driver_write(text);
}
