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

void error(char *file, int line, char *fmt, ...) {
  fprintf(stderr, "Error at %s:%d : ", file, line);
  va_list va;
  va_start(va, fmt);
  vfprintf(stderr, fmt, va);
  va_end(va);
  exit(1);
}

int eval_op(char* op, int x, int y) {
  ASSERT(op, "op is null");
  
  if (op[1] == 0) {
    switch (op[0]) {
    case '+': return x + y;
    case '-': return x - y;
    case '*': return x * y;
    case '/': return x / y;
    }
  }
  ERROR("unsupported operation: %s", op);
  return 0;
}

int eval (mpc_ast_t *ast) {
  ASSERT(ast, "ast is null");
    
  if (strstr(ast->tag, "number"))
    return atoi(ast->contents);

  ASSERT(ast->children_num > 2, "children number is too small");
  
  mpc_ast_t **children = ast->children;
  char *op = children[1]->contents;
  int max_idx = ast->children_num - 1;
  int result = eval(children[2]);
  
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
            "number   : /-?[0-9]/ ;                              \
             operator : '+' | '-' | '*' | '/' ;                  \
             expr     : <number> | '(' <operator> <expr> + ')' ; \
             root     : /^/ <operator> <expr> + /$/ ;",
            number, operator, expr, root);
  
  puts("Ba-bah scheme, C-c to exit\n");
  while (1) {
    char *input = readline("Ba-bah> ");
    add_history(input);

    mpc_result_t r;
    if (mpc_parse("<stdin>", input, root, &r)) {
      /* On Success Print the AST */
      //mpc_ast_print(r.output);
      printf("%d\n", eval(r.output));
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







