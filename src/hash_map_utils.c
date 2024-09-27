#include "hash_map_utils.h"

#include <math.h>
#include <assert.h>


double compute_load_factor(int num_keys, int bucket_size) {
    return (double)num_keys / bucket_size;
}

/*
 * Initializes the map to a given `size`.
 */
void init_map(hash_map_t *map, int size, int (*hash_fn)(void *),
              int (*compute_index)(void *, int), int (*key_equals)(void *, void *),
              int (*probe_fn)(int, int, int), void (*insert_fn)(void *, void *)) {
    map->primary = calloc(size, sizeof(void *));
    map->map_size = size;
    map->hash_function = hash_fn;
    map->compute_index = compute_index;
    map->key_equals = key_equals;
    map->probe_fn = probe_fn;
    map->insert_fn = insert_fn;
}

/*
 * Finds the next available index for a given key using a probing function.
 * If no probing function is defined, returns -1.
 */
int find_next_available(hash_map_t *map, void *key) {
    if (map->probe_fn == NULL) {
        printf("[DEBUG] hash_map_utils.c::find_next_available\tNo probing function defined!\n");
        return -1;
    }

    int index = map->compute_index(key, map->map_size);
    int i = 0;

    while (map->primary[index] != NULL) {
        index = map->probe_fn(index, i, map->map_size);
        i++;
    }

    return index;
}

/*
 * Searches a map that implements incremental rehashing.
 * It is necessary to check both the old and new maps
 * when searching for a key.
 *
 * Returns NULL if no such key was found.
 */
hash_element_t *search(hash_map_t *map, void *key) {
    if (map->temp != NULL) {
        int index = map->compute_index(key, map->temp_size);
        if (map->temp[index] != NULL && map->key_equals(map->temp[index]->key, key)) {
            return map->temp[index];
        }
    }
    if (map->primary != NULL) {
        int index = map->compute_index(key, map->map_size);
        hash_element_t *elem = map->primary[index];
        if (!elem) {
            return NULL;
        }
        if (map->key_equals(map->primary[index]->key, key)) {
            return map->primary[index];
        }
    }

    return NULL;
}

/*
 * Performs incremental rehashing on the map.
 */
void rehash_inc(hash_map_t *map) {
    for (int i = 0; i < R_VALUE; i++) {
        // If there is nothing to rehash at the moment, return immediately.
        if (map->temp == NULL) {
            return;
        }

        // Iterate to next non-null value
        while (map->temp_index < map->temp_size && map->temp[map->temp_index] == NULL) {
            map->temp_index++;
        }

        // Check if temp map is empty
        if (map->temp_index == map->temp_size) {
            free(map->temp);
            map->temp = NULL;
            map->temp_index = 0;

            return;
        }

        hash_element_t *elem = map->temp[map->temp_index];
        int index = map->compute_index(elem->key, map->map_size);

        // Collision resolution via linear probing
        if (map->primary[index]) {
            printf("[DEBUG] Rehashing collision detected!\n");

            if (map->probe_fn != NULL) {
              index = find_next_available(map, elem->key);
            }
        }

        printf("[DEBUG] Rehashing item into %d\n", index);

        assert(map->insert_fn != NULL);
        map->insert_fn(&map->primary[index], elem);
        map->primary_keys++;

        map->temp[map->temp_index] = NULL;
        map->temp_keys--;
    }
}

/*
 * Assumes that the element does not currently exist.
 */
void insert(hash_map_t *map, hash_element_t *elem) {
    int index = map->compute_index(elem->key, map->map_size);

    double factor = compute_load_factor(map->primary_keys + 1, map->map_size);

    printf("[DEBUG] factor = %.2lf\n", factor);

    if (factor >= LOAD_FACTOR) {
        printf("[DEBUG] Beginning rehash...\n");
        // Begin rehash
        map->temp = map->primary;
        map->temp_size = map->map_size;
        map->temp_keys = map->primary_keys;
        map->primary_keys = 0;
        map->primary = calloc(map->map_size * 2, sizeof(void *));
        map->map_size *= 2;
    }

    if (map->primary[index]) {
        printf("[DEBUG] Collision detected!\n");

        if (map->probe_fn != NULL) {
            index = find_next_available(map, elem->key);
        }
    }

    printf("[DEBUG] Inserting item at %d\n", index);

    assert(map->insert_fn != NULL);
    map->insert_fn(&map->primary[index], elem);
    map->primary_keys++;

    rehash_inc(map);
}

/*
 * Bulk inserts a list of elements into the map.
 * If the map is currently in the middle of a rehash,
 * sequential inserts will be done until the temporary
 * map is empty before adding the rest in bulk.
 * 
 * If the map is not in the middle of a rehash, the
 * elements will be inserted in bulk.
 */

void bulk_insert(hash_map_t *map, array_s *elems) {
    int unique = 0;
    int duplicate = 0;

    for (int i = 0; i < elems->numel; i++){ //iterating through the array
        hash_element_t *element = elems->data[i]; //taking the current element
        hash_element_t *existing = search (map, element->key); //checking if the element already exists
        if (existing == NULL){ //if the element doesn't exist, insert it into map
            insert (map, element);
            unique ++;
        }
        else{
            duplicate ++;
        }
    }

    printf("\n%d unique entries to be added (%d duplicates) out of %d.\n", unique, duplicate, elems->numel);

    //Performing rehashing when necessary
    if (unique > 0){
        printf ("Rehashing.....\n");
        rehash_inc (map);
    }
    printf ("Import Complete.\n");
}



