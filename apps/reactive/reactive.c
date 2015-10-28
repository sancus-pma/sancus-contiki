#include "reactive.h"

#include "log.h"
#include "connections.h"
#include "events.h"

#include <contiki-net.h>

#include <sancus_support/sm_control.h>
#include <sancus_support/tools.h>
#include <sancus_support/global_symtab.h>

#include <stdio.h>

typedef enum
{
    Ok                = 0x0,
    ErrIllegalCommand = 0x1,
    ErrPayloadFormat  = 0x2,
    ErrInternal       = 0x3
} ResultCode;

typedef struct
{
    // Should be a ResultCode. Stored as uint8_t to make sure it has the
    // correct size.
    uint8_t code;
    size_t  payload_size;
    void*   payload;
} Result;

#define RESULT(code)                 (Result){code, 0, NULL}
#define RESULT_DATA(code, len, data) (Result){code, len, data}

typedef enum
{
    EntrySetKey = 0x0
} SpecialEntryIds;

static struct psock socket;
static uint8_t socket_buffer[16];

struct
{
    Command command;
    uint8_t* payload;
    size_t payload_size;
    size_t bytes_read;
    size_t bytes_left;
    Result result;
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
        return RESULT(ErrPayloadFormat);
    if (!parse_int(state, &connection.from_output))
        return RESULT(ErrPayloadFormat);
    if (!parse_int(state, &connection.to_sm))
        return RESULT(ErrPayloadFormat);
    if (!parse_raw_data(state, sizeof(uip_ipaddr_t), (uint8_t**)&parsed_addr))
        return RESULT(ErrPayloadFormat);
    if (!parse_int(state, &connection.to_input))
        return RESULT(ErrPayloadFormat);

    connection.to_address = *parsed_addr;

    if (!connections_add(&connection))
        return RESULT(ErrInternal);

    LOG("added connection from SM %u output %u "
        "to SM %u at %u.%u.%u.%u input %u\n",
        connection.from_sm, connection.from_output, connection.to_sm,
        uip_ipaddr_to_quad(&connection.to_address), connection.to_input);

    return RESULT(Ok);
}

Result handle_set_key(ParseState* state)
{
    LOG("handling 'set_key' command\n");

    // The payload format is [sm_id, 16 bit nonce, index, wrapped(key), tag]
    // where the tag includes the nonce and the index.
    sm_id id;
    if (!parse_int(state, &id))
        return RESULT(ErrPayloadFormat);

    // We do not need to parse the nonce and the index since the verification
    // procedure inside the SM would need to transform it back to a buffer
    // anyway in order to pass it as associated data to the unwrap instruction.
    uint8_t* ad;
    const size_t AD_LEN = 2 + sizeof(io_index);
    if (!parse_raw_data(state, AD_LEN, &ad))
        return RESULT(ErrPayloadFormat);

    uint8_t* cipher;
    if (!parse_raw_data(state, SANCUS_KEY_SIZE, &cipher))
        return RESULT(ErrPayloadFormat);

    uint8_t* tag;
    if (!parse_raw_data(state, SANCUS_TAG_SIZE, &tag))
        return RESULT(ErrPayloadFormat);

    // The result format is [16 bit result code, tag] where the tag includes the
    // nonce and the result code.
    const size_t RESULT_PAYLOAD_SIZE = 2 + SANCUS_TAG_SIZE;
    void* result_payload = malloc(RESULT_PAYLOAD_SIZE);

    if (result_payload == NULL)
        return RESULT(ErrInternal);

    uint16_t args[] = {(uint16_t)ad, (uint16_t)cipher, (uint16_t)tag,
                       (uint16_t)result_payload};

    if (!sm_call_id(id, EntrySetKey, args, sizeof(args) / sizeof(args[0]),
                    /*retval=*/NULL))
    {
        free(result_payload);
        return RESULT(ErrInternal);
    }

    return RESULT_DATA(Ok, RESULT_PAYLOAD_SIZE, result_payload);
}

static Result handle_post_event(ParseState* state)
{
    // The packet format is [sm_id input_id data]
    sm_id sm;
    if (!parse_int(state, &sm))
        return RESULT(ErrPayloadFormat);

    io_index input;
    if (!parse_int(state, &input))
        return RESULT(ErrPayloadFormat);

    uint8_t* payload;
    size_t payload_len;
    if (!parse_all_raw_data(state, &payload, &payload_len))
        return RESULT(ErrPayloadFormat);

    reactive_handle_input(sm, input, payload, payload_len);
    return RESULT(Ok);
}

Result handle_call(ParseState* state)
{
    LOG("handling 'call' command\n");

    // The payload format is [sm_id, index]
    sm_id id;
    if (!parse_int(state, &id))
        return RESULT(ErrPayloadFormat);

    uint16_t index;
    if (!parse_int(state, &index))
        return RESULT(ErrPayloadFormat);

    if (!sm_call_id(id, index, NULL, 0, NULL))
        return RESULT(ErrInternal);

    return RESULT(Ok);
}

static command_handler command_handlers[] = {
    [Connect]   = handle_connect,
    [SetKey]    = handle_set_key,
    [PostEvent] = handle_post_event,
    [Call]      = handle_call
};

static PT_THREAD(handle_connection(struct psock* p))
{
    PSOCK_BEGIN(p);

    current_session.payload = NULL;
    current_session.result = RESULT(Ok);

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
        current_session.result.code = ErrIllegalCommand;
        goto end;
    }

    // Read the payload.
    current_session.payload = malloc(current_session.payload_size);

    if (current_session.payload == NULL)
    {
        LOG("out of memory\n");
        current_session.result.code = ErrInternal;
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
    current_session.result = command_handlers[current_session.command](state);
    free_parse_state(state);

end:
    PSOCK_SEND(p, &current_session.result.code,
               sizeof(current_session.result.code));
    PSOCK_SEND(p, current_session.result.payload,
               current_session.result.payload_size);

    free(current_session.payload);
    free(current_session.result.payload);

    PSOCK_END(p);
}

PROCESS(reactive_process, "Reactive process");

PROCESS_THREAD(reactive_process, ev, data)
{
    PROCESS_BEGIN();

    reactive_events_init();
    add_global_symbol("reactive_handle_output", &reactive_handle_output, NULL);

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
