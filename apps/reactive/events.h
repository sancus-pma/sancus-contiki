#ifndef REACTIVE_EVENTS_H
#define REACTIVE_EVENTS_H

#include "connections.h"

void reactive_events_init(void);

/**
 * API to be used by SMs.
 */
void reactive_handle_output(io_index output_id, void* data, size_t len);

/**
 * API to be used by the network layer.
 */
void reactive_handle_input(sm_id sm, io_index input, void* data, size_t len);

#endif
