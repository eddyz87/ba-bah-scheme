#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include <stdarg.h>
#include <search.h>

#include "mpc/mpc.h"
#include "utils.h"
#include "lisp_types.h"
#include "cons_pool.h"
#include "symbols.h"

/**
   Lisp symbols table:
     a pool of string pointers, feeled during read
     (and intern function, if im going to add one)
     
   Lisp environment:
     sorted (qsort) array of pairs (symbol*, LispValue)
     + size of this array
     + pointer to parent environment

   New environments are introduced by 'let' form:
     allocates array of (symbol*, value) pairs
     sorts this array, evaluates body with newly created environment
 */

#define DEF_PARSER(name) mpc_parser_t* name = mpc_new( #name )

void clean_lisp_value(LispValue lv) {
  if (is_cons(lv)) {
    Cons *cons = lv.value.cons;
    if (cons) {
      clean_lisp_value(cons->car);
      clean_lisp_value(cons->cdr);
      //free(cons);
    }
  } else if (is_error(lv)) {
    free(lv.value.error);
  }
}

void print_lisp_value(LispValue lv) {
  switch (lv.tag) {
  case FIXNUM_TAG:
    printf("%li", (long)lv.value.fixnum);
    break;
  case ERROR_TAG:
    printf("%s", lv.value.error);
    break;
  case SYMBOL_TAG:
    printf("%s", lv.value.symbol);
    break;
  case CONS_TAG: {
    Cons *cons = lv.value.cons;
    if (cons) {
      printf("(");
      print_lisp_value(cons->car);
      printf(" . ");
      print_lisp_value(cons->cdr);
      printf(")");
    } else {
      printf("Nil");
    }
    break;
  }
  }
}

#define MAX_FUNCTIONS 1000

typedef LispValue (*LispFunction) (LispValue);
static struct hsearch_data functions_table;

LispValue eval(LispValue lv);

typedef LispValue (*ArithmeticFunction) (long, long);

#define LASSERT_NOT_NULL(args)                        \
  do {                                                \
    if (is_null(args))                                \
      return make_error("Function argument is null"); \
  } while (0)

#define LASSERT_ERROR(args) \
  do {                      \
    if (is_error(args))     \
      return (args);        \
  } while (0)

#define LASSERT_CONS(args)                         \
  do {                                             \
    if (!is_cons(args))                            \
      return make_error("Argument is not a cons"); \
  } while (0)

LispValue apply_arithmetic_function(LispValue args, ArithmeticFunction func) {
  LASSERT_NOT_NULL(args);
  LASSERT_NOT_NULL(args.value.cons->cdr);
  
  LispValue x = eval(args.value.cons->car);
  LispValue y = eval(args.value.cons->cdr.value.cons->car);

  LASSERT_ERROR(x);
  LASSERT_ERROR(y);

  if (!is_fixnum(x) || !is_fixnum(y))
    return make_error("Arithmetic function argument is not a fixnum");

  return func(x.value.fixnum, y.value.fixnum);
}

LispValue af_add(long x, long y) { return make_fixnum(x + y); }
LispValue af_sub(long x, long y) { return make_fixnum(x - y); }
LispValue af_mul(long x, long y) { return make_fixnum(x * y); }
LispValue af_div(long x, long y) {
  if (y == 0)
    return make_error("Division by zero");
  return make_fixnum(x / y);
}

#define DEFINE_LF_FOR_AF(lf, af) \
  LispValue lf(LispValue args) { return apply_arithmetic_function(args, af); }

DEFINE_LF_FOR_AF(lf_add, af_add)
DEFINE_LF_FOR_AF(lf_sub, af_sub)
DEFINE_LF_FOR_AF(lf_mul, af_mul)
DEFINE_LF_FOR_AF(lf_div, af_div)

LispValue lf_quote(LispValue args) {
  return args;
}

LispValue force_arguments(LispValue args) {
  if (is_null(args))
    return args;

  LispValue x = eval(args.value.cons->car);
  if (is_error(x))
    return x;

  return make_cons(x, force_arguments(args.value.cons->cdr));
}

LispValue nth_arg(LispValue args, int n) {
  LASSERT_NOT_NULL(args);
  while (n > 0) {
    args = args.value.cons->cdr;
    LASSERT_NOT_NULL(args);
    n -= 1;
  }
  return args.value.cons->car;
}

LispValue _lf_car_or_cdr(LispValue args, int car) {
  args = force_arguments(args);
  LASSERT_ERROR(args);
  args = nth_arg(args, 0);
  LASSERT_ERROR(args);
  LASSERT_CONS(args);

  if (is_null(args))
    return make_null();

  if (car) {
    return args.value.cons->car;
  } else {
    return args.value.cons->cdr;
  }
}

LispValue lf_car(LispValue args) {
  return _lf_car_or_cdr(args, 1);
}

LispValue lf_cdr(LispValue args) {
  return _lf_car_or_cdr(args, 0);
}

LispValue lf_list(LispValue args) {
  return force_arguments(args);
}

LispValue lf_cons(LispValue args) {
  args = force_arguments(args);
  LASSERT_ERROR(args);

  LispValue x = nth_arg(args, 0);
  LASSERT_ERROR(x);

  LispValue xs = nth_arg(args, 1);
  LASSERT_ERROR(xs);

  return make_cons(x, xs);
}

LispValue lf_eval(LispValue args) {
  args = force_arguments(args);
  LASSERT_ERROR(args);
  
  LispValue x = nth_arg(args, 0);
  LASSERT_ERROR(x);

  return eval(x);
}

void cleanup_functions_table() {
  hdestroy_r(&functions_table);
}

void add_function(char *name, LispFunction func) {
  ENTRY ent = { name, func };
  ENTRY *p_ent;
  int result = hsearch_r(ent, ENTER, &p_ent, &functions_table);
  ASSERT(result != 0, "Can't add function %s to hash table, errcode: %d\n", name, errno);
}

LispFunction lookup_function(char *name) {
  ENTRY ent = { name, NULL };
  ENTRY *p_ent;
  int result = hsearch_r(ent, FIND, &p_ent, &functions_table);
  if (result != 0)
    return (LispFunction)p_ent->data;
  return NULL;
}

void init_functions_table() {
  hcreate_r(MAX_FUNCTIONS, &functions_table);
  add_function("+", lf_add);
  add_function("-", lf_sub);
  add_function("*", lf_mul);
  add_function("/", lf_div);
  add_function("quote", lf_quote);
  add_function("car", lf_car);
  add_function("cdr", lf_cdr);
  add_function("list", lf_list);
  add_function("cons", lf_cons);
  add_function("eval", lf_eval);
}

LispValue eval(LispValue lv) {
  if (!is_cons(lv) || is_null(lv))
    return lv;

  LispValue car = lv.value.cons->car;
  if (!is_symbol(car))
    return make_error("Function designator is not a symbol: %d", car.tag);
  
  LispFunction func = lookup_function(car.value.symbol);
  if (!func)
    return make_error("Unknown function: %s", car.value.symbol);

  return func(lv.value.cons->cdr);
}

LispValue read_lisp_value (mpc_ast_t *ast) {
  ASSERT(ast, "ast is null");

  //printf("Looking at tag: %s\n", ast->tag);
  
  // fixnum case
  if (strstr(ast->tag, "number")) {
    long val = strtol(ast->contents, NULL, 10);
    if (errno == ERANGE)
      return make_error("The value '%s' exceeds the range of fixnum values", ast->contents);
    else
      return make_fixnum(val);
  }

  // symbol case
  if (strstr(ast->tag, "symbol"))
    return make_symbol(ast->contents);
  
  // sexpr case
  if (strstr(ast->tag, "sexpr")) {
    mpc_ast_t **children = ast->children;
    int children_num = ast->children_num;
    
    if (children_num <= 2)
      return make_null();

    LispValue result = make_null();
    for (int i = children_num - 2; i > 0; --i) {
      result = make_cons(read_lisp_value(children[i]), result);
    }

    return result;
  }

  return make_error("Can't read s-expr: %s - %s'", ast->tag, ast->contents);
}

int main (int args, char *argv[]) {
  DEF_PARSER(number);
  DEF_PARSER(symbol);
  DEF_PARSER(sexpr);
  DEF_PARSER(expr);
  DEF_PARSER(root);

  mpca_lang(MPCA_LANG_DEFAULT,
            "number   : /-?[0-9]+/ ;"
            "symbol   : /[a-zA-Z+\\-*\\/][a-zA-Z0-9+\\-*\\/]*/ ;"
            "sexpr    : '(' <expr>* ')' ;"
            "expr     : <number> | <symbol> | <sexpr> ;"
            "root     : /^/ <expr> /$/ ;",
            number, symbol, sexpr, expr, root);

  init_functions_table();
  init_cons_allocator();
  init_symbols_table();

  puts("Ba-bah scheme, C-c to exit\n");

  while (1) {
    char *input = readline("Ba-bah> ");
    add_history(input);

    mpc_result_t r;
    if (mpc_parse("<stdin>", input, root, &r)) {
      //mpc_ast_print(r.output);
      mpc_ast_t *ast = r.output;
      ASSERT(ast->children_num > 1, "Malformed parsing result");
      LispValue lv = eval(read_lisp_value(ast->children[1]));
      print_lisp_value(lv);
      printf("\n");
      clean_lisp_value(lv);
      reset_cons_allocator();
      reset_symbols_table();
      mpc_ast_delete(r.output);
    } else {
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }
    
    free(input);
  }

  mpc_cleanup(5, number, symbol, sexpr, expr, root);
  cleanup_functions_table();
  cleanup_cons_allocator();
  cleanup_symbols_table();
  
  return 0;
}
