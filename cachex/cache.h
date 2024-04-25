//
// Created by Alex Brodsky on 2024-03-10.
//

#ifndef CACHE_CACHE_H
#define CACHE_CACHE_H

struct cache_info {
    void *F_memory;          /* pointer to "fast" memory that can be used by the cache */
    unsigned int F_size;   /* amount of "fast" memory (in bytes) */
    unsigned int M_size;   /* amount of main memory (in bytes) */
};

/* The following global variable and function are provided by main.c
 *   c_info MUST not be modified.  It is a read-only global variable.
 *   You may assume that the memory is initialized to all 0s.
 *   memget() is used to load the cache from "main memory"
 *     Parameters:
 *       address: address pf memory chuunk being requested
 *       buffer:  pointer to where the chunk of data from memory should be copied
 *       F_size:    F_size of the chunk in bytes
 *     Returns: 1 on success and 0 if address or (address + F_size) is out of range
 */
extern struct cache_info c_info;
extern unsigned int memget(unsigned int address, void *buffer, unsigned int size);

/* This function is called from main()
 * It simulates a cache query for an 8 byte value.  It takes two parameters:
 *   address: The address of the long value being feteched.
 *   value: a pointer to where the value should be stored.
 * Returns: 1 on aucces and 0 if the address is not in range of value is NULL.
 */
extern int cache_get(unsigned long address, unsigned long *value);
#endif //CACHE_CACHE_H
