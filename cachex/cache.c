
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "cache.h"

// struct representing a cache line
struct cache_lines {
    unsigned long tag;              // Tag to identify the memory block
    unsigned char data[32];         // Data stored in the cache line
    int valid;                      // Flag indicating if the cache line contains valid data
    int access;                     // Counter to track access time for implementing LRU policy
};

// struct representing a fully associative cache memory
struct cache_struct {
    struct cache_lines *lines;      // Array of cache lines
    int num_lines;                  // Number of cache lines
    int initialized;                // initialized flag
    int num_in_cache;               // Counter to track the number of accesses to the cache
};

// initialization of cache_struct
void init(struct cache_struct *cache_memory) {

    // Calculate the number of lines in the cache
    cache_memory->num_lines = (c_info.F_size - sizeof(struct cache_struct)) / sizeof(struct cache_lines);

    // Manually allocating memory for cache lines array
    cache_memory->lines = (struct cache_lines *)((char *)c_info.F_memory + sizeof(struct cache_struct));

    // Initializing the lines of the cache
    for (int i = 0; i < cache_memory->num_lines; ++i) {
        cache_memory->lines[i].tag = 0;
        for (int j = 0; j < sizeof(cache_memory->lines[i].data); ++j) {
            cache_memory->lines[i].data[j] = 0;
        }
        cache_memory->lines[i].valid = 0;
        cache_memory->lines[i].access = 0;
    }

    // Setting counter and initialized flag
    cache_memory->initialized = 1;
    cache_memory->num_in_cache = 0;
}

// Find the index of the least recently used cache line
int find_lru_index(struct cache_struct *cache_memory, unsigned long address,
        unsigned long offset, unsigned long tag, unsigned long *value, int n) {

    int lru_index = 0;
    int min_access_time = cache_memory->lines[0].access;

    // Iterate through cache lines to find the LRU line
    for (int i = 0; i < cache_memory->num_lines; i++) {
        if (cache_memory->lines[i].access < min_access_time) {
            min_access_time = cache_memory->lines[i].access;
            lru_index = i;
        }
    }

    // Load data from memory for the given LRU index found
    int size = memget(address - offset, cache_memory->lines[lru_index].data, 32);
    if (size) {
        cache_memory->lines[lru_index].tag = tag;
        cache_memory->lines[lru_index].valid = 1;
        cache_memory->lines[lru_index].access = cache_memory->num_in_cache; //update access time to current cache
        memcpy(value, &cache_memory->lines[lru_index].data[offset], n);
        return 1;
    }

    return 0;
}

extern int cache_get(unsigned long address, unsigned long *value) {
    // Check if cache is initialized, if not, initialize it
    struct cache_struct *cache_memory = c_info.F_memory;

    if (cache_memory->initialized == 0) {
        init(cache_memory);
    }
    cache_memory->num_in_cache++;

    // Calculate offset and tag from the memory address
    unsigned long offset = address & 0x1F;
    unsigned long tag =  address >> 5;

    int hit = -1;

    // Check if the memory access spans two cache lines
    if (offset > 24) {

        // Calculate address and tag for the second line
        unsigned long addressRight = address + (32 - offset);
        unsigned long tagRight = addressRight >> 5;

        int leftCached = 0;
        int rightCached = 0;
        int hitRight = -1;

        // Checking if memory is cached for both lines
        for (int i = 0; i < cache_memory->num_lines; i++) {
            if (cache_memory->lines[i].valid && cache_memory->lines[i].tag == tag) {
                hit = i;
                leftCached = 1;
            }
            if (cache_memory->lines[i].valid && cache_memory->lines[i].tag == tagRight) {
                hitRight = i;
                rightCached = 1;
            }
            if (hitRight != -1 && hit != -1) {
                break;
            }
        }

        // If both parts of the memory access are cached
        if (leftCached && rightCached) {
            cache_memory->lines[hit].access = cache_memory->num_in_cache;   //Update access time
            unsigned long valRight = 0;
            unsigned long valLeft = 0;

            // Copy both the cached data blocks into left and right values
            memcpy(&valLeft, &cache_memory->lines[hit].data[offset], 32 - offset);
            memcpy(&valRight, &cache_memory->lines[hitRight].data[0], sizeof(long) - (32 - offset));

            // Merge left and right values and load into *value
            unsigned long valueFinal = (valRight << (8 * (32 - offset))) | valLeft;
            memcpy(value, &valueFinal, sizeof(long));

            return 1;
        } else if (leftCached) {    // If only the left part of the memory access is cached

            cache_memory->lines[hit].access = cache_memory->num_in_cache;   //Update access time
            unsigned long valRight = 0;
            unsigned long valLeft = 0;

            // Copy the left cached data blocks into left value
            memcpy(&valLeft, &cache_memory->lines[hit].data[offset], 32 - offset);
            // Find the LRU for the right value
            int lru_index = find_lru_index(cache_memory, addressRight, 0, tagRight,
                                           &valRight, sizeof(long) - (32 - offset));

            // Merge left and right values and load into *value
            unsigned long valueFinal = (valRight << (8 * (32 - offset))) | valLeft;
            memcpy(value, &valueFinal, sizeof(long));

            return lru_index;


        } else if (rightCached) {   // If only the right part of the memory access is cached
            cache_memory->lines[hitRight].access = cache_memory->num_in_cache;      //Update access time
            unsigned long valRight = 0;
            unsigned long valLeft = 0;

            // Find the LRU for the left value
            int lru_index = find_lru_index(cache_memory, address, offset, tag, &valLeft, (32 - offset));
            // Copy the right cached data blocks into right value
            memcpy(&valRight, &cache_memory->lines[hitRight].data[0], sizeof(long) - (32 - offset));

            // Merge left and right values and load into *value
            unsigned long valueFinal = (valRight << (8 * (32 - offset))) | valLeft;
            memcpy(value, &valueFinal, sizeof(long));

            return lru_index;

        } else {    // If neither part of the memory access is cached
            unsigned long valRight = 0;
            unsigned long valLeft = 0;

            // Find the LRU for the right and left values
            int lru_index = find_lru_index(cache_memory, address, offset, tag, &valLeft, (32 - offset));
            int lru_index_Right = find_lru_index(cache_memory, addressRight, 0, tagRight,
                                                 &valRight, sizeof(long) - (32 - offset));

            // Merge left and right values and load into *value
            unsigned long valueFinal = (valRight << (8 * (32 - offset))) | valLeft;
            memcpy(value, &valueFinal, sizeof(long));

            return lru_index && lru_index_Right;
        }
    }

    // Check if the memory access is cached in a single cache line
    for (int i = 0; i < cache_memory->num_lines; i++) {
        if (cache_memory->lines[i].valid && cache_memory->lines[i].tag == tag) {
            hit = i;
            break;
        }
    }

    // If cache hit, update access time and return data
    if (hit != -1) {
        cache_memory->lines[hit].access = cache_memory->num_in_cache;
        memcpy(value, &cache_memory->lines[hit].data[offset], sizeof(long));

        return 1;
    }

    // If cache miss, find LRU cache line and load data from main memory
    int lru_index = find_lru_index(cache_memory, address, offset, tag, value, sizeof(long));

    return lru_index;
}
