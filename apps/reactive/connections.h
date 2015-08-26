#ifndef CONNECTIONS_H
#define CONNECTIONS_H

#include <sancus/sm_support.h>

#include "net/ip/uip.h"

typedef size_t io_index;

typedef struct
{
    sm_id        from_sm;
    io_index     from_output;
    sm_id        to_sm;
    uip_ipaddr_t to_address;
    io_index     to_input;
} Connection;

// Copies connection so may be stack allocated.
int connections_add(Connection* connection);

// We keep ownership of the returned Connection. May return NULL.
Connection* connections_get(sm_id from_sm, io_index from_output);


#endif
