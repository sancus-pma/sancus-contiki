#ifndef REACTIVE_EVENTS_H
#define REACTIVE_EVENTS_H

#include "connections.h"

/**
 * API to be used by SMs.
 */
void reactive_handle_output(io_index output_id, const void* data, size_t len);

/**
 * API to be used by the network layer.
 */
void reactive_handle_input(void* packet, size_t len);

#endif
