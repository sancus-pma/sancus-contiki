#include "events.h"

#include "reactive.h"
#include "log.h"
#include "connections.h"

#include <contiki-net.h>

#include <sancus/sm_support.h>

#include <sancus_support/sm_control.h>

#include <stdlib.h>

typedef struct
{
    Connection* connection;
    void*       data;
    size_t      len;
} Event;

static Event queued_event;
static int has_queued_event = 0;

PROCESS(reactive_remote_events_process, "Reactive remote events process");

void reactive_events_init()
{
    process_start(&reactive_remote_events_process, NULL);
}

static int is_local_connection(Connection* connection)
{
    return uip_ipaddr_cmp(&connection->to_address, &uip_hostaddr);
}

static void handle_local_connection(Connection* connection,
                                    void* data, size_t len)
{
    reactive_handle_input(connection->to_sm, connection->to_input, data, len);
    free(data);
}

static void handle_remote_connection(Connection* connection,
                                     void* data, size_t len)
{
    queued_event.connection = connection;
    queued_event.data = data;
    queued_event.len = len;
    has_queued_event = 1;
    process_poll(&reactive_remote_events_process);
}

void reactive_handle_output(io_index output_id, void* data, size_t len)
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

void reactive_handle_input(sm_id sm, io_index input, void* data, size_t len)
{
    uint16_t args[] = {input, (uint16_t)data, (uint16_t)len};
    sm_call_id(sm, 1, args, sizeof(args) / sizeof(args[0]), /*retval=*/NULL);
}

static PT_THREAD(send_remote_event(struct psock* p))
{
    static uint8_t* packet = NULL;
    static size_t packet_len = 0;

    PSOCK_BEGIN(p);

    // We send a packet to a remote reactive server. Therefore, the format is
    // the same as the one used in reactive.c:handle_connection:
    // [command length payload].
    // In our case, command is PostEvent and the payload format is
    // [sm_id input_id data]
    packet_len = 2 + 2 + 2 + 2 + queued_event.len;
    packet = malloc(packet_len);

    if (packet == NULL)
        LOG("out-of-memory while sending packet\n");
    else
    {
        uint16_t* command  = (uint16_t*)(packet);
        uint16_t* length   = (uint16_t*)(packet + 2);
        uint16_t* sm_id    = (uint16_t*)(packet + 4);
        uint16_t* input_id = (uint16_t*)(packet + 6);
        uint8_t*  data     =            (packet + 8);

        *command  = UIP_HTONS(PostEvent);
        *length   = uip_htons(2 + 2 + queued_event.len);
        *sm_id    = uip_htons(queued_event.connection->to_sm);
        *input_id = uip_htons(queued_event.connection->to_input);
        memcpy(data, queued_event.data, queued_event.len);

        PSOCK_SEND(p, packet, packet_len);

        free(packet);
        packet = NULL;
        packet_len = 0;
    }

    PSOCK_CLOSE_EXIT(p);
    PSOCK_END(p);
}

PROCESS_THREAD(reactive_remote_events_process, ev, data)
{
    static struct psock socket;
    static uint8_t socket_buffer[16];

    PROCESS_BEGIN();

    while (1)
    {
        PROCESS_WAIT_EVENT_UNTIL(has_queued_event);

        LOG("handling queued remote event\n");

        if (tcp_connect(&queued_event.connection->to_address,
                        UIP_HTONS(REACTIVE_PORT), NULL) == NULL)
        {
            LOG("failed to open TCP connection, retrying later\n");
            continue;
        }

        PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);

        if (uip_connected())
        {
            LOG("connected to remote reactive server at %u.%u.%u.%u:%u\n",
                uip_ipaddr_to_quad(&uip_conn->ripaddr), uip_conn->rport);

            PSOCK_INIT(&socket, socket_buffer, sizeof(socket_buffer));

            while (!(uip_aborted() || uip_closed() || uip_timedout()))
            {
                PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);

                send_remote_event(&socket);
            }

            LOG("connection closed\n");
        }
        else
        {
            LOG("failed to connect to remote reactive server at %u.%u.%u.%u, "
                "dropping event\n",
                uip_ipaddr_to_quad(&queued_event.connection->to_address));
        }

        free(queued_event.data);
        queued_event.connection = NULL;
        queued_event.data = NULL;
        queued_event.len = 0;
        has_queued_event = 0;
    }

    PROCESS_END();
}
