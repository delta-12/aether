#ifndef AETHER_HASHMAP_H
#define AETHER_HASHMAP_H

#include <stddef.h>
#include <stdint.h>

#include "err.h"

typedef struct Hashmap_Entry a_Hashmap_Entry_t;

typedef struct
{
    a_Hashmap_Entry_t **entries;
    size_t capacity;
    size_t size;
    size_t iterating;
} a_Hashmap_t;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

a_Err_t a_Hashmap_Initialize(a_Hashmap_t *const hashmap);
void a_Hashmap_Deinitialize(a_Hashmap_t *const hashmap);
a_Err_t a_Hashmap_Insert(a_Hashmap_t *const hashmap, const void *const key, const size_t key_size, const void *const value, const size_t value_size);
void *a_Hashmap_Get(const a_Hashmap_t *const hashmap, const void *const key, const size_t key_size);
a_Err_t a_Hashmap_Remove(a_Hashmap_t *const hashmap, const void *const key, const size_t key_size);
a_Err_t a_Hashmap_ForEach(a_Hashmap_t *const hashmap,
                          void (*callback)(const void *const key,
                                           const size_t key_size,
                                           void *const value,
                                           const size_t value_size,
                                           const void *const arg),
                          const void *const arg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AETHER_HASHMAP_H */
