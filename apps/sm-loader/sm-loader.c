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
    uint16_t   bytes_read;
    uint16_t   bytes_left;
    uint8_t*   buffer;
    sm_id      sm_id;
    ElfModule* sm_elf;
    size_t     num_symbols;
    size_t     current_symbol;
    char       symbol_buf[128];
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
    current_session.sm_elf = sm_get_elf_by_id(current_session.sm_id);
    free(current_session.buffer);

    uint16_t id_to_send = uip_htons(current_session.sm_id);
    PSOCK_SEND(p, (uint8_t*)&id_to_send, sizeof(id_to_send));

    current_session.num_symbols = symtab_get_num_symbols();

    // Send all global symbols.
    for (current_session.current_symbol = 0;
         current_session.current_symbol < current_session.num_symbols;
         current_session.current_symbol++)
    {
        Symbol symbol;
        int is_section;
        ElfModule* module;

        if (!symtab_get_symbol(current_session.current_symbol,
                               &symbol, &is_section, &module))
        {
            LOG("symtab error");
            PSOCK_CLOSE_EXIT(p);
        }

        if (!is_section && module == NULL)
        {
            size_t len = snprintf(current_session.symbol_buf,
                                  sizeof(current_session.symbol_buf),
                                  "%s = %p;\n", symbol.name, symbol.value);

            if (len > sizeof(current_session.symbol_buf))
            {
                LOG("symtab buffer too small\n");
                PSOCK_CLOSE_EXIT(p);
            }

            PSOCK_SEND_STR(p, current_session.symbol_buf);
        }
    }

    // Send module sections.
    PSOCK_SEND_STR(p, "SECTIONS\n{\n");

    for (current_session.current_symbol = 0;
         current_session.current_symbol < current_session.num_symbols;
         current_session.current_symbol++)
    {
        Symbol symbol;
        int is_section;
        ElfModule* module;

        if (!symtab_get_symbol(current_session.current_symbol,
            &symbol, &is_section, &module))
        {
            LOG("symtab error");
            PSOCK_CLOSE_EXIT(p);
        }

        if (is_section && module == current_session.sm_elf)
        {
            size_t len = snprintf(current_session.symbol_buf,
                                  sizeof(current_session.symbol_buf),
                                  "%s %p : {}\n", symbol.name, symbol.value);

            if (len > sizeof(current_session.symbol_buf))
            {
                LOG("symtab buffer too small");
                PSOCK_CLOSE_EXIT(p);
            }

            PSOCK_SEND_STR(p, current_session.symbol_buf);
        }
    }

    PSOCK_SEND_STR(p, "}\n");

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
