#ifndef SANCUS_COMMON_CONTIKI_CONF_H
#define SANCUS_COMMON_CONTIKI_CONF_H

/* Our clock resolution, this is the same as Unix HZ. */
#define CLOCK_CONF_SECOND 128UL

#define CCIF
#define CLIF

#define HAVE_STDINT_H
#include "msp430def.h"

/* Types for clocks and uip_stats */
typedef unsigned short uip_stats_t;
typedef unsigned long clock_time_t;
typedef unsigned long off_t;

#endif
