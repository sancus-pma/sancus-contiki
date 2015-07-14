#include <sancus_support/uart.h>

int putchar(int c)
{
    if (c == '\n')
        putchar('\r');

    uart_write_byte(c);
    return c;
}

