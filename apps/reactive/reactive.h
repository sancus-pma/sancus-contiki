#ifndef REACTIVE_H
#define REACTIVE_H

#include <sys/process.h>

#include <stddef.h>

PROCESS_NAME(reactive_process);

typedef enum
{
    Connect   = 0x0,
    SetKey    = 0x1,
    PostEvent = 0x2,
    Call      = 0x3,
    CommandsEnd
} Command;

#endif
