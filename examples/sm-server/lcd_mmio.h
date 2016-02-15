#ifndef PORT1_MMIO_H
#define PORT1_MMIO_H

#include <stdint.h>

#include <sancus/sm_support.h>

extern struct SancusModule lcd_mmio;

void SM_ENTRY(lcd_mmio) lcd_mmio_init(void);
char SM_ENTRY(lcd_mmio) lcd_mmio_write_byte(char b);

#endif
