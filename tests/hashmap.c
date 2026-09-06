#ifndef HASHMAP_IMPLEMENTATION
#define HASHMAP_IMPLEMENTATION
#endif

#include "../hashmap.h"

#include<stdio.h>
#include<time.h>

#define N 100

int main(void) {
  hashmap_t *map = hashmap_new();

  srand(time(NULL));

  int keys[N] = {0};
  int values[N] = {0};

  for(int i = 0; i < N; i++) {
    keys[i] = i;
    values[i] = rand();
  }

  for(int i = 0; i < N; i++) {
    hashmap_put(map, (map_entry_t) {
      .key = (char*)((int*)keys + i),
      .key_length = sizeof(int),
      .value = values + i
    });
  }

  for(int i = 20; i < 50; i++) {
    hashmap_remove(map, (char*)((int*)keys + i), sizeof(int));
  }

  for(int i = 0; i < N; i++) {
    if(hashmap_contains(map, (char*)((int*)keys + i), sizeof(int))) {
      printf("Contains value for key '%d': '%d'.\n",
             i, *(int*)hashmap_get(map, (char*)((int*)keys + i), sizeof(int)));
    } else {
      printf("Does not contain value for key '%d'.\n", i);
    }
  }

  hashmap_destroy(map);
  return 0;
}
