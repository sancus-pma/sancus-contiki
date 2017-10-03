#include "ipaddr.h"

// These bytes where picked because they seem to be very infrequently used in
// most MSP430 ELF files. Therefore, this sequence has a very low probability of
// occurring anywhere else in the ELF file.
uip_ipaddr_t ipaddr = {{'\xf5', '\xd7', '\xc5', '\xbd'}};
