#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "cache.h"

struct cache_info c_info;
static void *memory;
static int hits;
static int misses;
static int miss;

static void log_result(unsigned long address) {
    hits += !miss;
    misses += miss != 0;
#ifdef DEBUG
    static char * result[] = {"miss", "hit"};
    printf("Cache %s @ 0x%8.8lx\n", result[!miss], address);
#endif
}

int main() {
    setbuf(stdout, 0);

    if (scanf("%d", &c_info.F_size) != 1) {
        printf("Error reading fast memory size\n");
        return 0;
    }
    c_info.F_memory = calloc(1, c_info.F_size);
    assert(c_info.F_memory);

    if (scanf("%d", &c_info.M_size) != 1) {
        printf("Error reading memory size\n");
        return 0;
    }

    memory = calloc(c_info.M_size + sizeof(long), 1);
    assert(memory);
    srandom(0xc0ffeed);
    for (int i = 0; i < c_info.M_size; i += 4) {
        *(unsigned long *)(memory + i) = random();
    }

    int num_refs = 0;
    if (scanf("%d", &num_refs) != 1) {
        printf("Error reading number of references\n");
        return 0;
    }

    for (int i = 0; i < num_refs; i++) {
        unsigned int address;
        if (scanf("%u", &address) != 1) {
            printf("Error reading operation\n");
            return 0;
        }

        assert(address <= c_info.M_size);

        unsigned long word;
        miss = 0;
        cache_get(address, &word);
        unsigned long expected = *(unsigned long *)(memory + address);

        if (word != expected) {
            printf("Error reading memory address 0x%8.8x\n", address);
            printf("  Expected 0x%16.16lx\n", expected);
            printf("  Actual 0x%16.16lx\n", word);
            return 0;
        }

        log_result(address);

        printf("Loaded value [0x%16.16lx] @ address 0x%8.8x\n", word, address);
    }

    char buffer[10];
    if (scanf("%9s", buffer) == 1 && !strcmp(buffer, "stats")) {
        printf("Cache hits: %d, misses: %d -- hit rate %d%%\n", hits, misses, 100 * hits / num_refs);
    }
    return 0;
}

extern unsigned int memget(unsigned int address, void *buffer, unsigned int size) {
    if (address + size > c_info.M_size) {
        size = c_info.M_size - address;
    }
    memcpy(buffer, memory + address, size);
    miss++;
    return size;
}
