#include <stdlib.h>

#include "lisp_types.h"
#include "cons_pool.h"

#define CONS_POOL_SIZE 1024

typedef struct _ConsPool {
  Cons conses[CONS_POOL_SIZE];
  int free_index;
  struct _ConsPool *next_pool;
} ConsPool;

typedef struct {
  ConsPool *root;
  ConsPool *current;
} ConsAllocator;

static ConsAllocator cons_allocator;

static ConsPool *new_cons_pool() {
  return calloc(1, sizeof(ConsPool));
}

void init_cons_allocator() {
  cons_allocator.root = cons_allocator.current = new_cons_pool();
}

static void free_cons_pool(ConsPool *pool) {
  while(pool) {
    ConsPool *next = pool->next_pool;
    free(pool);
    pool = next;
  }
}

void reset_cons_allocator() {
  cons_allocator.root->free_index = 0;
  free_cons_pool(cons_allocator.root->next_pool);
}

void cleanup_cons_allocator() {
  free_cons_pool(cons_allocator.root);
}

Cons *new_cons() {
  if (cons_allocator.current->free_index < CONS_POOL_SIZE) {
    return &cons_allocator.current->conses[cons_allocator.current->free_index ++];
  }
  ConsPool *pool = new_cons_pool();
  pool->free_index = 1;
  cons_allocator.current->next_pool = pool;
  cons_allocator.current = pool;
  return &pool->conses[0];
}
