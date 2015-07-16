#include "networking.h"

#include "net/ip/uip.h"
#include "dev/slip.h"

#include <sancus_support/uart.h>

void slip_arch_writeb(unsigned char b)
{
    uart_write_byte(b);
}

static void slip_input_byte_wrapper(unsigned char b)
{
    slip_input_byte(b);
}

void networking_init()
{
    uip_init();
    uip_ipaddr_t hostaddr, netmask;
    uip_ipaddr(&hostaddr, 192, 168, 0, 2);
    uip_ipaddr(&netmask, 255, 255, 255, 0);
    uip_sethostaddr(&hostaddr);
    uip_setnetmask(&netmask);
    uart_set_receive_cb(slip_input_byte_wrapper);
    tcpip_set_outputfunc(slip_send);

    process_start(&tcpip_process, NULL);
    process_start(&slip_process, NULL);
}
