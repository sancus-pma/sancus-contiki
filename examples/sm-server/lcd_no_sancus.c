#include "lcd.h"

#include <sancus_support/pmodcls.h>

void lcd_init()
{
    pmodcls_init();
}

void lcd_write(const char* text)
{
    pmodcls_write(text);
}
