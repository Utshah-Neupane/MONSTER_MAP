#ifndef HASH_MAP_UTILS_H_
#define HASH_MAP_UTILS_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "array_utils.h"

#define LOAD_FACTOR 0.75
#define R_VALUE 1

#define CALCULATE_BUCKET_SIZE(num_keys, LOAD_FACTOR) (ceil((num_keys)/(LOAD_FACTOR))) 



typedef struct {
    void *key;
    void *value;
} hash_element_t;

typedef struct {
    hash_element_t **temp;
    hash_element_t **primary;
    int primary_keys;
    int temp_keys;
    int temp_size;
    int map_size;
    int temp_index;

    int (*hash_function)(void *);
    int (*compute_index)(void *, int);
    // Should return 1 if the keys are equal, 0 otherwise
    int (*key_equals)(void *, void *);
    int (*probe_fn)(int, int, int);
    void (*insert_fn)(void *, void *);
    void (*free_fn)(void *);
} hash_map_t;

void init_map(hash_map_t *,
              int,
              int (*hash_fn)(void *),
              int (*compute_index)(void *, int),
              int (*key_equals)(void *, void *),
              int (*probe_fn)(int, int, int),
              void (*insert_fn)(void *, void *));

int find_next_available(hash_map_t *, void *);
hash_element_t *search(hash_map_t *, void *);
void rehash_inc(hash_map_t *);
void insert(hash_map_t *, hash_element_t *);
void bulk_insert(hash_map_t *, array_s *);

#endif
