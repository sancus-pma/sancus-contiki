#ifndef SM_SERVER_LCD_DRIVER_H
#define SM_SERVER_LCD_DRIVER_H

#include <sancus/sm_support.h>

SM_ENTRY(lcd_driver) void lcd_driver_init(void);
SM_ENTRY(lcd_driver) void lcd_driver_acquire(void);
SM_ENTRY(lcd_driver) void lcd_driver_release(void);
SM_ENTRY(lcd_driver) void lcd_driver_clear(void);
SM_ENTRY(lcd_driver) void lcd_driver_write(const char* text);

extern struct SancusModule lcd_driver;

#endif
