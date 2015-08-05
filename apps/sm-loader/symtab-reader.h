#ifndef SYMTAB_READER_H
#define SYMTAB_READER_H

#include <sancus/sm_support.h>

// Returns an NTBS that should be freed by the caller.
char* read_symtab_for_sm(sm_id id);

#endif
