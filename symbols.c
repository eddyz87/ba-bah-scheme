#include <search.h>
#include <errno.h>

#include "lisp_types.h"
#include "symbols.h"
#include "utils.h"

#define MAX_SYMBOLS (16*1024)

static struct hsearch_data symbols_table;

void init_symbols_table() {
  hcreate_r(MAX_SYMBOLS, &symbols_table);
}

void cleanup_symbols_table() {
  hdestroy_r(&symbols_table);
}

void reset_symbols_table() {
  cleanup_symbols_table();
  init_symbols_table();
}

Symbol intern_symbol(char *name) {
  ENTRY ent = { name, 0 };
  ENTRY *p_ent;
  int result;

  result = hsearch_r(ent, FIND, &p_ent, &symbols_table);
  if (result == 0) {
    ASSERT(errno == ESRCH, "Unexpected error while searching for symbol %s, code is: %d\n",
           name, errno);

    result = hsearch_r(ent, ENTER, &p_ent, &symbols_table);
    ASSERT(result != 0, "Unexpected error while adding symbol %s, code is: %d\n",
           name, errno);
  }
  
  return (Symbol)p_ent->key;
}


