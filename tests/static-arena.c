#include<stdio.h>
#include<assert.h>

#ifndef STATIC_ARENA_IMPLEMENTATION
#define STATIC_ARENA_IMPLEMENTATION
#endif

#include "../static-arena.h"

int main(void) {
  static_arena_t *arena = st_arena_new(KB(1));
  int *x = st_arena_alloc(arena, 10, sizeof(int));

  if(x == NULL) {
    printf("An unexpected error occured when allocating the arena of %.3fKB.\n",
           (float)st_arena_capacity(arena) / KB(1));
    return 1;
  }

  printf("Free space in the arena: %zu\n", st_arena_available(arena));
  assert(st_arena_alloc(arena, KB(5), 1) == NULL); // No enough memory available
  
  // Resets the arena:
  st_arena_reset(arena);
  // Now 'x' becomes a dangling pointer
  char *s = st_arena_alloc(arena, 1, 1);
  printf("Free space in the arena: %zu\n", st_arena_available(arena));

  st_arena_destroy(&arena);
  return 0;
}
