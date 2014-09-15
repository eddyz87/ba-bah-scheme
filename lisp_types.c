#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "lisp_types.h"

#define LISP_ERROR_MESSAGE_SIZE 256

LispValue make_fixnum(long val) {
  LispValue lv;
  lv.tag = FIXNUM_TAG;
  lv.value.fixnum = val;
  return lv;
}

LispValue make_error(char *fmt, ...) {
  char *desc = malloc(LISP_ERROR_MESSAGE_SIZE);
  va_list va;
  va_start(va, fmt);
  vsnprintf(desc, LISP_ERROR_MESSAGE_SIZE, fmt, va);
  va_end(va);

  LispValue lv;
  lv.tag = ERROR_TAG;
  lv.value.error = desc;
  return lv;
}

LispValue make_symbol(char *name) {
  LispValue lv;
  lv.tag = SYMBOL_TAG;
  lv.value.symbol = intern_symbol(name);
  return lv;
}

LispValue make_cons(LispValue car, LispValue cdr) {
  LispValue lv;
  lv.tag = CONS_TAG;
  lv.value.cons = new_cons();
  lv.value.cons->car = car;
  lv.value.cons->cdr = cdr;
  return lv;
}

LispValue make_null() {
  LispValue lv;
  lv.tag = CONS_TAG;
  lv.value.cons = NULL;
  return lv;
}

int check_tag(LispValue lv, int tag) {
  return lv.tag == tag;
}


#define DEF_TAG_CHECK(func, tag) \
  int func(LispValue lv) {       \
    return check_tag(lv, (tag)); \
  }

DEF_TAG_CHECK(is_error  , ERROR_TAG)
DEF_TAG_CHECK(is_fixnum , FIXNUM_TAG)
DEF_TAG_CHECK(is_cons   , CONS_TAG)
DEF_TAG_CHECK(is_symbol , SYMBOL_TAG)

int is_null(LispValue lv) {
  return is_cons(lv) && !lv.value.cons;
}
