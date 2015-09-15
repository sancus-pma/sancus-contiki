#include "symtab-reader.h"

#include <sancus_support/global_symtab.h>
#include <sancus_support/sm_control.h>

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static const size_t BUFFER_INIT_SIZE = 1024;
static const size_t BUFFER_INCREMENT = 128;

static char* buffer = NULL;
static size_t buffer_size = 0, buffer_index = 0;

static int increase_buffer_size()
{
    if (buffer == NULL)
    {
        buffer = malloc(BUFFER_INIT_SIZE);

        if (buffer == NULL)
            return 0;

        buffer_size = BUFFER_INIT_SIZE;
        return 1;
    }

    size_t new_size = buffer_size + BUFFER_INCREMENT;
    char* new_buffer = malloc(new_size);

    if (new_buffer == NULL)
        return 0;

    memcpy(new_buffer, buffer, buffer_size);
    free(buffer);
    buffer = new_buffer;
    buffer_size = new_size;
    return 1;
}

static int buf_putchar(int c)
{
    if (buffer_index >= buffer_size)
    {
        if (!increase_buffer_size())
            return EOF;
    }

    buffer[buffer_index++] = c;
    return c;
}

static int print_cb(const char* str, ...)
{
    va_list ap;
    va_start(ap, str);
    int ret = vuprintf(buf_putchar, str, ap);
    va_end(ap);
    return ret;
}

char* read_symtab_for_sm(sm_id id)
{
    print_global_symbols(print_cb);
    print_module_sections(sm_get_elf_by_id(id), print_cb);
    buf_putchar('\0');

    char* result = buffer;
    buffer = NULL;
    buffer_size = buffer_index = 0;
    return result;
}
