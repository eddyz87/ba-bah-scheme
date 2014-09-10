#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include <stdarg.h>
#include "mpc/mpc.h"

#define DEF_PARSER(name) mpc_parser_t* name = mpc_new( #name )
/*
#define IPRINTF(level, fmt_string, ...)               \
  printf ("%*s" fmt_string, (level), "", __VA_ARGS__)
*/
#define ERROR(fmt, ...) error(__FILE__, __LINE__, (fmt), ##__VA_ARGS__)
#define ASSERT(cond, fmt, ...)                \
  do {                                        \
    if (!(cond)) { ERROR(fmt, ##__VA_ARGS__); } \
  } while(0)

enum {
  FIXNUM_TAG = 0x0,
  ERROR_TAG = 0x1,
};

enum {
  ERR_DIV_BY_ZERO,
  ERR_BAD_OP,
  ERR_RANGE,
};

typedef struct {
  int tag:2;
  union {
    long fixnum:62;
    long err_code:62;
  } value;
} LispValue;

LispValue make_fixnum(long val) {
  LispValue lv;
  lv.tag = FIXNUM_TAG;
  lv.value.fixnum = val;
  return lv;
}

LispValue make_error(long code) {
  LispValue lv;
  lv.tag = ERROR_TAG;
  lv.value.err_code = code;
  return lv;
}

int check_tag(LispValue lv, int tag) {
  return lv.tag == tag;
}

int is_error(LispValue lv) {
  return check_tag(lv, ERROR_TAG);
}

void print_lisp_value(LispValue lv) {
  switch (lv.tag) {
  case FIXNUM_TAG:
    printf("%li\n", (long)lv.value.fixnum);
    break;
  case ERROR_TAG:
    switch (lv.value.err_code) {
    case ERR_BAD_OP: printf("Bad operation\n"); break;
    case ERR_DIV_BY_ZERO: printf("Division by zero\n"); break;
    case ERR_RANGE: printf("Value out of range\n"); break;
    }
    break;
  }
}

void error(char *file, int line, char *fmt, ...) {
  fprintf(stderr, "Error at %s:%d : ", file, line);
  va_list va;
  va_start(va, fmt);
  vfprintf(stderr, fmt, va);
  va_end(va);
  exit(1);
}

LispValue eval_op(char* op, LispValue x, LispValue y) {
  ASSERT(op, "op is null");
  if (is_error(x)) return x;
  if (is_error(y)) return y;

  long xv = x.value.fixnum;
  long yv = y.value.fixnum;

  if (op[1] == 0) {
    switch (op[0]) {
    case '+': return make_fixnum(xv + yv);
    case '-': return make_fixnum(xv - yv);
    case '*': return make_fixnum(xv * yv);
    case '/':
      if (y.value.fixnum == 0)
        return make_error(ERR_DIV_BY_ZERO);
      else
        return make_fixnum(xv / yv);
    }
  }

  return make_error(ERR_BAD_OP);
}

LispValue eval (mpc_ast_t *ast) {
  ASSERT(ast, "ast is null");
    
  if (strstr(ast->tag, "number")) {
    long val = strtol(ast->contents, NULL, 10);
    if (errno == ERANGE)
      return make_error(ERR_RANGE);
    else
      return make_fixnum(val);
  }

  ASSERT(ast->children_num > 2, "children number is too small");
  
  mpc_ast_t **children = ast->children;
  char *op = children[1]->contents;
  int max_idx = ast->children_num - 1;
  LispValue result = eval(children[2]);
  
  for (int i = 3; i < max_idx; ++i)
    result = eval_op(op, result, eval(children[i]));

  return result;
}

int main (int args, char *argv[]) {
  DEF_PARSER(number);
  DEF_PARSER(operator);
  DEF_PARSER(expr);
  DEF_PARSER(root);

  mpca_lang(MPCA_LANG_DEFAULT,
            "number   : /-?[0-9]+/ ;                             \
             operator : '+' | '-' | '*' | '/' ;                  \
             expr     : <number> | '(' <operator> <expr> + ')' ; \
             root     : /^/ <operator> <expr>+ /$/ ;",
            number, operator, expr, root);

  puts("Ba-bah scheme, C-c to exit\n");
  while (1) {
    char *input = readline("Ba-bah> ");
    add_history(input);

    mpc_result_t r;
    if (mpc_parse("<stdin>", input, root, &r)) {
      /* On Success Print the AST */
      //mpc_ast_print(r.output);
      print_lisp_value(eval(r.output));
      mpc_ast_delete(r.output);
    } else {
      /* Otherwise Print the Error */
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }
    
    free(input);
  }

  mpc_cleanup(4, number, operator, expr, root);
  
  return 0;
}







