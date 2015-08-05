#include "symtab-reader.h"

#include "contiki-net.h"

#include <sancus_support/sm_control.h>
#include <sancus_support/tools.h>
#include <sancus_support/global_symtab.h>

#include <stdio.h>

#define LOG(...) printf("SM loader: " __VA_ARGS__)

static struct psock socket;
static uint8_t socket_buffer[64];

struct
{
    uint16_t bytes_read;
    uint16_t bytes_left;
    uint8_t* buffer;
    sm_id    sm_id;
    char*    symtab_data;
} current_session;

static sm_id load_sm_from_buffer()
{
    if (current_session.buffer == NULL)
        return 0;

    if (current_session.bytes_left != 0)
    {
        LOG("received incomplete packet (expected %u more bytes)",
            current_session.bytes_left);
        return 0;
    }

    sm_id id = 0;
    ParseState* state = create_parse_state(current_session.buffer,
                                           current_session.bytes_read);

    char* name;
    if (!parse_string(state, &name))
    {
        LOG("packet format error: name\n");
        goto out;
    }

    vendor_id vid;
    if (!parse_int(state, &vid))
    {
        LOG("packet format error: VID\n");
        goto out;
    }

    uint8_t* sm_file;
    size_t sm_file_size;
    parse_all_raw_data(state, &sm_file, &sm_file_size);

    LOG("loading SM with name %s and VID %u\n", name, vid);
    id = sm_load(sm_file, name, vid);

    if (id == 0)
    {
        LOG("error loading SM file\n");
        goto out;
    }

    LOG("successfully loaded SM with ID %u\n", id);

out:
    free_parse_state(state);
    return id;
}

static PT_THREAD(handle_connection(struct psock* p))
{
    PSOCK_BEGIN(p);

    // The packet format is [LEN NAME \0 VID ELF_FILE]
    // LEN is the length of the packet without LEN itself
    // Parse the length
    PSOCK_READBUF_LEN(p, 2);
    uint16_t packet_size = uip_ntohs(*(uint16_t*)socket_buffer);
    void* session_buffer = malloc(packet_size);

    if (session_buffer == NULL)
    {
        LOG("not enough memory to store packet\n");
        PSOCK_CLOSE_EXIT(p);
    }

    // socket_buffer might contain more than the requested 2 bytes so copy the
    // remaining bytes first.
    uint16_t bytes_left = PSOCK_DATALEN(p) - 2;
    memcpy(session_buffer, socket_buffer + 2, bytes_left);

    current_session.buffer = session_buffer;
    current_session.bytes_read = bytes_left;
    current_session.bytes_left = packet_size - bytes_left;
    current_session.sm_id = 0;
    current_session.symtab_data = NULL;

    while (current_session.bytes_left > 0)
    {
        PSOCK_READBUF_LEN(p, current_session.bytes_left);
        uint16_t len = PSOCK_DATALEN(p);
        void* dest = current_session.buffer + current_session.bytes_read;
        memcpy(dest, socket_buffer, len);
        current_session.bytes_read += len;
        current_session.bytes_left -= len;
    }

    current_session.sm_id = load_sm_from_buffer();
    free(current_session.buffer);

    uint16_t id_to_send = uip_htons(current_session.sm_id);
    PSOCK_SEND(p, (uint8_t*)&id_to_send, sizeof(id_to_send));

    current_session.symtab_data = read_symtab_for_sm(current_session.sm_id);
    PSOCK_SEND(p, (uint8_t*)current_session.symtab_data,
                  strlen(current_session.symtab_data) + 1);

    PSOCK_CLOSE(p);
    PSOCK_END(p);
}

PROCESS(sm_loader_process, "SM loader process");

PROCESS_THREAD(sm_loader_process, ev, data)
{
    PROCESS_BEGIN();

    LOG("started on %u.%u.%u.%u:%u\n",
        uip_ipaddr_to_quad(&uip_hostaddr), SM_LOADER_PORT);

    tcp_listen(UIP_HTONS(SM_LOADER_PORT));

    while (1)
    {
        PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);

        if (uip_connected())
        {
            LOG("connection from %u.%u.%u.%u:%u\n",
                uip_ipaddr_to_quad(&uip_conn->ripaddr), uip_conn->rport);

            PSOCK_INIT(&socket, socket_buffer, sizeof(socket_buffer));
            current_session.buffer = NULL;
            current_session.bytes_read = 0;
            current_session.bytes_left = 0;

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
