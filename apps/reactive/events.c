#include "events.h"

#include "log.h"
#include "connections.h"

#include <sancus/sm_support.h>

#include <sancus_support/sm_control.h>

static int is_local_connection(Connection* connection)
{
    return uip_ipaddr_cmp(&connection->to_address, &uip_hostaddr);
}

static void handle_local_connection(Connection* connection,
                                    const void* data, size_t len)
{
    uint16_t args[] = {connection->to_input, (uint16_t)data, (uint16_t)len};

    sm_call_id(connection->to_sm, 1,
               args, sizeof(args) / sizeof(args[0]), /*retval=*/NULL);
}

static void handle_remote_connection(Connection* connection,
                                     const void* data, size_t len)
{
    LOG("remote connections not supported yet");
}

void reactive_handle_output(io_index output_id, const void* data, size_t len)
{
    sm_id calling_sm = sancus_get_caller_id();

    Connection* connection = connections_get(calling_sm, output_id);

    if (connection == NULL)
    {
        LOG("no connection for output %u:%u\n", calling_sm, output_id);
        return;
    }

    LOG("accepted output %u:%u to be delivered at %u.%u.%u.%u:%u:%u\n",
        connection->from_sm, connection->from_output,
        uip_ipaddr_to_quad(&connection->to_address),
        connection->to_sm, connection->to_input);

    if (is_local_connection(connection))
        handle_local_connection(connection, data, len);
    else
        handle_remote_connection(connection, data, len);
}

void reactive_handle_input(void* packet, size_t len)
{

}
