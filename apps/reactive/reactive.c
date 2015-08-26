#include "contiki-net.h"

#include "connections.h"

#include <sancus_support/sm_control.h>
#include <sancus_support/tools.h>

#include <stdio.h>

#define LOG(...) printf("Reactive: " __VA_ARGS__)

typedef enum
{
    Connect   = 0x0,
    SetKey    = 0x1,
    PostEvent = 0x2,
    CommandsEnd
} Command;

typedef enum
{
    Ok                = 0x0,
    ErrIllegalCommand = 0x1,
    ErrPayloadFormat  = 0x2,
    ErrInternal       = 0x3
} Result;

static struct psock socket;
static uint8_t socket_buffer[16];

struct
{
    Command command;
    uint8_t* payload;
    size_t payload_size;
    size_t bytes_read;
    size_t bytes_left;
} current_session;

typedef Result (*command_handler)(ParseState*);

Result handle_connect(ParseState* state)
{
    LOG("handling 'connect' command\n");

    // The payload format is [from_sm from_output to_sm to_address to_input]
    // which is basically this same thing as a Connection.
    Connection connection;
    uip_ipaddr_t* parsed_addr;

    if (!parse_int(state, &connection.from_sm))
        return ErrPayloadFormat;
    if (!parse_int(state, &connection.from_output))
        return ErrPayloadFormat;
    if (!parse_int(state, &connection.to_sm))
        return ErrPayloadFormat;
    if (!parse_raw_data(state, sizeof(uip_ipaddr_t), (uint8_t**)&parsed_addr))
        return ErrPayloadFormat;
    if (!parse_int(state, &connection.to_input))
        return ErrPayloadFormat;

    connection.to_address = *parsed_addr;
    connections_add(&connection);

    LOG("added connection from SM %u output %u "
        "to SM %u at %u.%u.%u.%u input %u\n",
        connection.from_sm, connection.from_output, connection.to_sm,
        uip_ipaddr_to_quad(&connection.to_address), connection.to_input);

    return Ok;
}

Result handle_set_key(ParseState* state)
{
    return Ok;
}

Result handle_post_event(ParseState* state)
{
    return Ok;
}

static command_handler command_handlers[] = {
    [Connect]   = handle_connect,
    [SetKey]    = handle_set_key,
    [PostEvent] = handle_post_event
};

static PT_THREAD(handle_connection(struct psock* p))
{
    PSOCK_BEGIN(p);

    uint8_t result = Ok;

    // The common packet format is [command length payload] where command and
    // length are both 2 bytes and length refers to the size of the payload.
    // We start with receiving/parsing the header.
    const size_t HEADER_SIZE = 4;
    PSOCK_READBUF_LEN(p, HEADER_SIZE);
    ParseState* state = create_parse_state(socket_buffer, HEADER_SIZE);
    parse_int(state, &current_session.command);
    parse_int(state, &current_session.payload_size);
    free_parse_state(state);

    if (current_session.command >= CommandsEnd)
    {
        LOG("illegal command: %x\n", current_session.command);
        result = ErrIllegalCommand;
        goto end;
    }

    // Read the payload.
    current_session.payload = malloc(current_session.payload_size);

    if (current_session.payload == NULL)
    {
        LOG("out of memory\n");
        result = ErrInternal;
        goto end;
    }

    // First copy the remaining bytes in the buffer.
    size_t bytes_left = PSOCK_DATALEN(p) - HEADER_SIZE;
    memcpy(current_session.payload, socket_buffer + HEADER_SIZE, bytes_left);
    current_session.bytes_read = bytes_left;
    current_session.bytes_left = current_session.payload_size - bytes_left;

    // Then receive the rest.
    while (current_session.bytes_left > 0)
    {
        PSOCK_READBUF_LEN(p, current_session.bytes_left);
        size_t len = PSOCK_DATALEN(p);
        void* dest = current_session.payload + current_session.bytes_read;
        memcpy(dest, socket_buffer, len);
        current_session.bytes_read += len;
        current_session.bytes_left -= len;
    }

    // Call the correct handler.
    state = create_parse_state(current_session.payload,
                               current_session.payload_size);
    result = command_handlers[current_session.command](state);
    free_parse_state(state);

end:
    PSOCK_SEND(p, &result, sizeof(result));
    PSOCK_END(p);
}

PROCESS(reactive_process, "Reactive process");

PROCESS_THREAD(reactive_process, ev, data)
{
    PROCESS_BEGIN();

    LOG("started on %u.%u.%u.%u:%u\n",
        uip_ipaddr_to_quad(&uip_hostaddr), REACTIVE_PORT);

    tcp_listen(UIP_HTONS(REACTIVE_PORT));

    while (1)
    {
        PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);

        if (uip_connected())
        {
            LOG("connection from %u.%u.%u.%u:%u\n",
                uip_ipaddr_to_quad(&uip_conn->ripaddr), uip_conn->rport);

            PSOCK_INIT(&socket, socket_buffer, sizeof(socket_buffer));

            while (!(uip_aborted() || uip_closed() || uip_timedout()))
            {
                PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);

                handle_connection(&socket);
            }

            LOG("connection closed\n");
        }
    }

    PROCESS_END();
}
