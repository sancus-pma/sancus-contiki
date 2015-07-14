#include <msp430.h>

int putchar(int c)
{
    P1OUT = c;
    P1OUT |= 0x80;
    return c;
}

