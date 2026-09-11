/* @author: eduardo-lamounier
 * @date: 06/09/2026 [DD/MM/YYYY]
 *
 * A header-only library containing the implementation of a dynamically
 * allocated hash map/table, which uses open adressing.
 *
 * To use it, put this before including the file:
 * #define HASHMAP_IMPLEMENTATION
 *
 * WARN: Make sure this definition is only made in ONE source file that's being
 * compiled.
 */

#ifndef HASHMAP_H
#define HASHMAP_H

#include<string.h>
#include<stdlib.h>
#include<stdint.h>
#include<stdbool.h>

typedef struct {
  const char *key;
  uint64_t key_length;
  void *value;
} map_entry_t;

typedef struct hashmap hashmap_t;

// Returns a reference to the created hash map.
//
// Returns NULL if it isn't possible to create it.
hashmap_t *hashmap_new(void);

// Releases all resources of a hash map.
//
// The pointer to the hashmap becomes invalid.
void hashmap_destroy(hashmap_t *map);

// Puts a new entry to the hashmap.
//
// Returns whether it was possible to add the new entry. The operation
// can fail if rehashing fails consecutivelly to allocate new space in
// memory, making the hash table full in a certain point.
//
// If the entry's key is already associated to a value, then the value is just
// updated for the specified entry's value.
//
// The pointer to the entry's key and value MUST be valid as long as the
// hashmap is alive.
bool hashmap_put(hashmap_t *map, map_entry_t entry);

// Removes the entry associated to the specified key from the hash table.
//
// If no entry is associated to the specified key, returns `false`. Otherwise,
// returns `true`.
bool hashmap_remove(hashmap_t *map, const char *key, uint64_t key_length);

// Returns whether there's an entry associated to the specified key in the hash
// table.
bool hashmap_contains(hashmap_t *map, const char *key, uint64_t key_length);

// Gets the value associated with a specified key. Returns `NULL` if the key
// isn't associated with any entry.
//
// If your data can be `NULL` and you are not sure whether a value was
// associated with the specified key, you should:
//   1 - Use `hashmap_contains(...)` to verify whether the key is in the
//   hashmap - if so, this function will only return `NULL` for the key if the
//   value itself is `NULL`;
//   2 - Use `hashmap_get_entry(...)`, which will return `NULL` if the key 
//   isn't in the hashmap or a valid reference to the entry - which will have
//   the real value associated to the key.
void *hashmap_get(hashmap_t *map, const char *key, uint64_t key_length);

// Gets a reference to the entry associated with a specified key. Returns
// 'NULL' if the key isn't associated with any entry.
map_entry_t *hashmap_get_entry(hashmap_t *map, const char *key, uint64_t key_length);

#endif



#ifdef HASHMAP_IMPLEMENTATION

#include<math.h>
#include<assert.h>
#include<inttypes.h>

#define STARTING_CAPACITY 16

#define HIGH_LOAD_FACTOR 0.7
#define LOW_LOAD_FACTOR  0.5

#define FNV_OFFSET_BASIS 0xcbf29ce484222325
#define FNV_PRIME        0x00000100000001b3

struct hashmap {
  map_entry_t *entries;
  bool *tombstones;
  uint64_t capacity;
  uint64_t used;
};

void try_to_rehash(hashmap_t *map) {
  uint64_t count = 0;
  for(uint64_t i = 0; i < map->capacity; i++)
    count += map->entries[i].key != NULL && !map->tombstones[i]; 

  uint64_t capacity = map->capacity;
  while((double)count > LOW_LOAD_FACTOR * capacity)
    capacity *= 2;

  hashmap_t new_map = {
    .entries = (map_entry_t*)calloc(capacity, sizeof(map_entry_t)),
    .tombstones = (bool*)calloc(capacity, sizeof(bool)),
    .capacity = capacity,
    .used = 0,
  };

  if(new_map.entries == NULL || new_map.tombstones == NULL) {
    if(new_map.entries != NULL) {
      free(new_map.entries);
    }
    
    if(new_map.tombstones != NULL) {
      free(new_map.tombstones);
    }

    return;
  }

  for(uint64_t i = 0; i < map->capacity; i++) {
    if(map->entries[i].key != NULL && !map->tombstones[i]) {
      hashmap_put(&new_map, map->entries[i]);
    }
  }

  free(map->entries);
  *map = new_map;
}

// FNV-1a
uint64_t hashf(const char *key, uint64_t key_length) {
  uint64_t hash = FNV_OFFSET_BASIS;

  for(uint64_t i = 0; i < key_length; i++) {
    hash ^= (unsigned char)key[i];
    hash *= FNV_PRIME;
  }

  return hash;
}

static inline bool eq_keys(const char *k1, const char *k2, uint64_t k1_len, uint64_t k2_len) {
  if(k1_len == 0 && k2_len == 0) {
    return true;
  }

  return k1_len == k2_len && k1[0] == k2[0] && !strncmp(k1, k2, k1_len);
}

/******************************************************************************/

hashmap_t *hashmap_new(void) {
  hashmap_t *map = (hashmap_t*)calloc(1, sizeof(hashmap_t));

  if(map == NULL) {
    return NULL;
  }

  map->capacity = STARTING_CAPACITY;
  map->entries = (map_entry_t*)calloc(map->capacity, sizeof(map_entry_t));
  map->tombstones = (bool*)calloc(map->capacity, sizeof(bool));

  if(map->entries == NULL || map->tombstones == NULL) {
    if(map->entries != NULL) {
      free(map->entries);
    }

    if(map->tombstones != NULL) {
      free(map->tombstones);
    }

    free(map);
    return NULL;
  }
  
  return map;
}

void hashmap_destroy(hashmap_t *map) {
  assert(map != NULL);
  free(map->entries);
  free(map->tombstones);
  free(map);
}

bool hashmap_put(hashmap_t *map, map_entry_t entry) {
  assert(map != NULL);

  if((double)map->used / map->capacity > HIGH_LOAD_FACTOR) { 
    try_to_rehash(map);
  }

  uint64_t begin_idx = hashf(entry.key, entry.key_length) % map->capacity;

  for(uint64_t i = 0; i < map->capacity; i++) {
    uint64_t idx = (begin_idx + i) % map->capacity;

    if(map->entries[idx].key == NULL) {
      map->entries[idx] = entry;
      map->used++;
      return true;
    }
    
    if(!map->tombstones[idx] &&
      eq_keys(map->entries[idx].key, entry.key, 
              map->entries[idx].key_length, entry.key_length)
    ) {
      map->entries[idx].value = entry.value;
      return true;
    }
  }

  return false;
}

bool hashmap_remove(hashmap_t *map, const char *key, uint64_t key_length) {
  assert(map != NULL && key != NULL);

  uint64_t begin_idx = hashf(key, key_length) % map->capacity;

  for(uint64_t i = 0; i < map->capacity; i++) {
    uint64_t idx = (begin_idx + i) % map->capacity;

    if(map->entries[idx].key == NULL) {
      return false;
    }

    if(!map->tombstones[idx] &&
       eq_keys(map->entries[idx].key, key,
               map->entries[idx].key_length, key_length)
    ) {
      map->tombstones[idx] = true;
      map->used--;
      return true;
    }
  }

  return false;
}

bool hashmap_contains(hashmap_t *map, const char *key, uint64_t key_length) {
  assert(map != NULL && key != NULL);

  uint64_t begin_idx = hashf(key, key_length) % map->capacity;

  for(uint64_t i = 0; i < map->capacity; i++) {
    uint64_t idx = (begin_idx + i) % map->capacity;

    if(map->entries[idx].key == NULL) {
      return false;
    }
    
    if(eq_keys(map->entries[idx].key, key, 
               map->entries[idx].key_length, key_length)) {
      return !map->tombstones[idx];
    }
  }

  return false;
}

void *hashmap_get(hashmap_t *map, const char *key, uint64_t key_length) {
  map_entry_t *entry = hashmap_get_entry(map, key, key_length); 

  return entry != NULL ? entry->value : NULL;
}

map_entry_t *hashmap_get_entry(hashmap_t *map, const char *key, uint64_t key_length) {
  assert(map != NULL && key != NULL);

  uint64_t begin_idx = hashf(key, key_length) % map->capacity;

  for(uint64_t i = 0; i < map->capacity; i++) {
    uint64_t idx = (begin_idx + i) % map->capacity;

    if(map->entries[idx].key == NULL) {
      return NULL;
    }

    if(!map->tombstones[idx] &&
       eq_keys(map->entries[idx].key, key,
               map->entries[idx].key_length, key_length)
    ) {
      return map->entries + idx;   
    }
  }

  return NULL;
}

#undef STARTING_CAPACITY

#undef HIGH_LOAD_FACTOR
#undef LOW_LOAD_FACTOR

#undef FNV_OFFSET_BASIS
#undef FNV_PRIME

#endif
