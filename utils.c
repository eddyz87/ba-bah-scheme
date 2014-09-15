#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "utils.h"

void error(char *file, int line, char *fmt, ...) {
  fprintf(stderr, "Error at %s:%d : ", file, line);
  va_list va;
  va_start(va, fmt);
  vfprintf(stderr, fmt, va);
  va_end(va);
  exit(1);
}
