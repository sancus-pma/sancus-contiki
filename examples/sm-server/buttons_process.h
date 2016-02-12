#ifndef BUTTONS_PROCESS_H
#define BUTTONS_PROCESS_H

#include <sys/process.h>

PROCESS_NAME(buttons_process);

void buttons_process_poll(void);

#endif
