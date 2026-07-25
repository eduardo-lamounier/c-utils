/* @author: eduardo-lamounier
 * @date: 25/07/2026 [DD/MM/YYYY]
 *
 * A header-only file containing the implementation of fixed-size memory
 * arenas. 
 *
 * To use it, put this before including the file:
 * #define STATIC_ARENA_IMPLEMENTATION
*/

#ifndef STATIC_ARENA_H
#define STATIC_ARENA_H

#include<stdlib.h>

#define KB(x) (   (x) * (size_t)1024 )
#define MB(x) ( KB(x) * (size_t)1024 )
#define GB(x) ( MB(x) * (size_t)1024 )

#define MAX_STATIC_ARENAS 256

typedef struct arena static_arena_t;

// Creates a new arena and returns a pointer to it.
//
// The arena is initialized with the specified amount of bytes, but if it isn't
// possible to allocate that memory, NULL is returned.
//
// Also, if the amount of simultaneously used arenas reaches 
// 'MAX_STATIC_ARENAS', NULL is returned.
//
// The arena must be destroyed after use with 'st_arena_destroy'.
//
// Also returns NULL if 'capacity' is zero.
static_arena_t *st_arena_new(size_t capacity);

// Releases all resources from an arena, freeing everything that was allocated
// in it.
//
// Receives a pointer to the arena's pointer (aka arena's pointer is passed by
// reference), making the pointer to the arena NULL - as the arena is now
// unavailable.
void st_arena_destroy(static_arena_t **arena);

// Doesn't destroy the arena, instead, ignores everything once allocated in it
// and acts like a brand-new empty arena with the same capacity. Still needs to
// be destroyed with 'st_arena_destroy'.
void st_arena_reset(static_arena_t *arena);

// Allocates memory for the specified 'n' amount of elements of the specified
// size each. Returns a pointer to the beginning of the allocated memory.
//
// It's guaranteed that the allocated memory will be completely zero-filled.
//
// If the arena is full or doesn't have enough room for this amount of memory,
// NULL is returned.
//
// Also returns NULL if either 'n' or 'size' are zero.
void *st_arena_alloc(static_arena_t *arena, size_t n, size_t size);

#endif

#ifdef STATIC_ARENA_IMPLEMENTATION

#include<assert.h>
#include<stdint.h>

struct arena {
  char *data;
  size_t capacity;
  size_t offset;
};

typedef struct {
  bool used;
  static_arena_t arena;
} pool_obj_t;

pool_obj_t object_pool[MAX_STATIC_ARENAS];

// Borrows an arena struct from the object pool. If there's no position of the
// object pool free, returns NULL; otherwise, the position of the arena
// returned is marked as not free, decreasing the room left for more arenas.
inline static static_arena_t *borrow_arena(void) {
  for(int i = 0; i < MAX_STATIC_ARENAS; i++)
    if(!object_pool[i].used) {
      object_pool[i].used = true;
      return &object_pool[i].arena;
    }

  return NULL;
}

// Tells the object pool to mark the position of the returned arena as free.
inline static void return_arena(static_arena_t *arena) {
  assert(arena != NULL);
  size_t i = (uintptr_t)arena - (uintptr_t)object_pool;

  assert(object_pool[i].used);
  object_pool[i].used = false;
}

// ============================================================================

static_arena_t *st_arena_new(size_t capacity) {
  if(capacity == 0)
    return NULL;

  static_arena_t *arena = borrow_arena();

  if(arena == NULL)
    return NULL;

  arena->offset = 0;
  arena->capacity = capacity;
  arena->data = (char*)calloc(capacity, 1);

  if(arena->data == NULL) {
    free(arena);
    return NULL;
  }

  return arena;
}

void st_arena_destroy(static_arena_t **arena) {
  assert(arena != NULL && *arena != NULL);
  free((*arena)->data);
  return_arena(*arena);
  *arena = NULL;
}

void st_arena_reset(static_arena_t *arena) {
  assert(arena != NULL);
  arena->offset = 0;
}

void *st_arena_alloc(static_arena_t *arena, size_t n, size_t size) {
  assert(arena != NULL);
  assert(n != 0 && size != 0);
  arena->offset = (arena->offset + 8-1) & ~(8-1);
  
  if(arena->offset + n * size >= arena->capacity)
    return NULL;

  void *addr = arena->data + arena->offset;
  arena->offset += n * size;
  return addr;
}

#endif
