#include <sancus_support/uart.h>

int putchar(int c)
{
    if (c == '\n')
        putchar('\r');

    uart2_write_byte(c);
    return c;
}
