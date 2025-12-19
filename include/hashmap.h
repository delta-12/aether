#ifndef AETHER_HASHMAP_H
#define AETHER_HASHMAP_H

#include <stddef.h>
#include <stdint.h>

#include "err.h"

typedef struct
{
    uint8_t *data;
    size_t size;
    size_t key_size;
    size_t value_size;
} a_Hashmap_t;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

a_Err_t a_Hashmap_Initialize(a_Hashmap_t *const hashmap, uint8_t *const data, const size_t size, const size_t key_size, const size_t value_size);
a_Err_t a_Hashmap_Insert(a_Hashmap_t *const hashmap, const void *const key, const void *const value);
void *a_Hashmap_Get(a_Hashmap_t *const hashmap, const void *const key);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AETHER_HASHMAP_H */
