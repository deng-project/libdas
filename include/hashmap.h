/// libdas: DENG asset handling management library
/// licence: Apache, see LICENCE file
/// file: hashmap.h - simple C hashmap header
/// author: Karl-Mihkel Ott

#ifndef HASHMAP_H
#define HASHMAP_H

#ifdef __HASHMAP_C
    #include <stdlib.h>
    #include <stdint.h>
    #include <stdbool.h>
    #include <string.h>
    #include <stdio.h>
    #include <limits.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __HashData {
    void *data;
    void *key;
    size_t key_len;
} __HashData;

typedef struct __Hashmap {
    __HashData *map_data;
    int64_t *indices;
    size_t map_size;
    size_t used_size;
} Hashmap;

#ifdef __HASHMAP
    /* 
     * Hashing function for hashmapping that returns index
     * This function is based on mostly Jenkins one at time hashing algorihm
     * Steps to finding index are following:
     * 1. Find crc32_key from key data
     * 2. Perform Jenkins one at time bitwise operations
     * 3. Perform three other Jenkins operations
     * 4. Multiply bit-shifted out_key with constant 0x9E3779B1
     * 5. Return hash % n
     */
    static size_t __hashfunc(void *key, size_t n_key, size_t map_size); 


    /// Reallocate more memory for buckets in hashmap 
    static void __reallocateHashmap(Hashmap *p_hm);

    
    /*
     * Key comparisson method
     * Returns 0 if keys are equal, 1 if key1 is longer than key2 and -1 if 
     * key2 is longer than key1 then returns 1 and 2 if keys are the same length
     * but their memory areas do not match
     */
    static int __keycmp(void *key1, size_t n1, void *key2, size_t n2);


    /// Search for the bucket index of the key element
    static size_t __findIndex(Hashmap *p_hm, void *key, size_t key_size);
#endif

    /// Create a new hashmap instance
    void newHashmap(Hashmap *p_hashmap, size_t n_len);


    /// Push value to the hashmap
    void pushToHashmap(Hashmap *p_hm, void *key, size_t key_size, void *val);


    /// Pop the value from hashmap that is specified with the key
    void *popFromHashmap(Hashmap *p_hm, void *key, size_t key_size);


    /// Find the list of all elements' pointers in hashmap
    void **getHashmapList(Hashmap *p_hm);


    /// Find value from the map by specified key
    void *findValue(Hashmap *p_hm, void *key, size_t key_len);

    
    /// Destroy the given hashmap instance
    void destroyHashmap(Hashmap *p_hm);

#ifdef __cplusplus
}
#endif

#endif
