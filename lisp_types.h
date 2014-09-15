#ifndef __LISP_TYPES_H__
#define __LISP_TYPES_H__

enum {
  FIXNUM_TAG  = 0x0,
  ERROR_TAG   = 0x1,
  CONS_TAG    = 0x2,
  SYMBOL_TAG  = 0x3,
};

typedef char* Symbol;

struct _Cons;

typedef struct {
  unsigned int tag:2;
  union {
    long fixnum;
    char* error;
    Symbol symbol;
    struct _Cons *cons;
  } value;
} LispValue;

typedef struct _Cons {
  LispValue car;
  LispValue cdr;
} Cons;

Symbol intern_symbol(char*);
Cons *new_cons();
LispValue make_fixnum(long val);
LispValue make_error(char *fmt, ...);
LispValue make_symbol(char *name);
LispValue make_cons(LispValue car, LispValue cdr);
LispValue make_null();

int check_tag(LispValue lv, int tag);
int is_error(LispValue lv);
int is_fixnum(LispValue lv);
int is_cons(LispValue lv);
int is_symbol(LispValue lv);
int is_null(LispValue lv);

#endif /*__LISP_TYPES_H__*/

