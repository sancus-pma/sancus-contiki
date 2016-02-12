#ifndef PORT1_MMIO_H
#define PORT1_MMIO_H

#include <stdint.h>

#include <sancus/sm_support.h>

typedef uint64_t port1_data_t;

extern struct SancusModule port1_mmio;

port1_data_t SM_ENTRY(port1_mmio) port1_mmio_read(void);
void         SM_ENTRY(port1_mmio) port1_mmio_write(port1_data_t data);

#define R_P1IN(data)  (uint8_t)(((data) >>  0) & 0xff)
#define R_P1OUT(data) (uint8_t)(((data) >>  8) & 0xff)
#define R_P1DIR(data) (uint8_t)(((data) >> 16) & 0xff)
#define R_P1IFG(data) (uint8_t)(((data) >> 24) & 0xff)
#define R_P1IES(data) (uint8_t)(((data) >> 32) & 0xff)
#define R_P1IE(data)  (uint8_t)(((data) >> 40) & 0xff)
#define R_P1SEL(data) (uint8_t)(((data) >> 48) & 0xff)

#define W_P1IN(byte)  ((uint64_t)((byte) & 0xff) <<  0)
#define W_P1OUT(byte) ((uint64_t)((byte) & 0xff) <<  8)
#define W_P1DIR(byte) ((uint64_t)((byte) & 0xff) << 16)
#define W_P1IFG(byte) ((uint64_t)((byte) & 0xff) << 24)
#define W_P1IES(byte) ((uint64_t)((byte) & 0xff) << 32)
#define W_P1IE(byte)  ((uint64_t)((byte) & 0xff) << 40)
#define W_P1SEL(byte) ((uint64_t)((byte) & 0xff) << 48)

#endif
