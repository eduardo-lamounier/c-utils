#include<stdio.h>

#ifndef DYNAMIC_ARENA_IMPLEMENTATION
#define DYNAMIC_ARENA_IMPLEMENTATION
#endif

#include "../dynamic-arena.h"

int main(void) {
  dynamic_arena_t *arena = dy_arena_new(KB(10));

  const char *text = "Hello world!";

  char *allocated = dy_arena_alloc_copy(
    arena, 
    50,
    text,
    strlen(text)+1
  );

  printf("|%s|\n", allocated);

  dy_arena_destroy(&arena);
  return 0;
}
