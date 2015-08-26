#include "sm-loader.h"
#include "reactive.h"

#include "sys/autostart.h"

AUTOSTART_PROCESSES(&sm_loader_process, &reactive_process);
