#include "hashmap.h"

#include <stddef.h>
#include <stdint.h>

#include "err.h"

a_Err_t a_Hashmap_Initialize(a_Hashmap_t *const hashmap, uint8_t *const data, const size_t size, const size_t key_size, const size_t value_size)
{
    a_Err_t error = A_ERR_NONE;

    if ((NULL == hashmap) || (NULL == data))
    {
        error = A_ERR_NULL;
    }
    else if ((0U == size) || (0U == key_size) || (0U == value_size))
    {
        error = A_ERR_SIZE;
    }
    else
    {
        /* TODO */
    }

    return error;
}

a_Err_t a_Hashmap_Insert(a_Hashmap_t *const hashmap, const void *const key, const void *const value)
{
    a_Err_t error = A_ERR_NULL;

    if ((NULL != hashmap) && (NULL != key) && (NULL != value))
    {
        /* TODO */

        error = A_ERR_NONE;
    }

    return error;
}

void *a_Hashmap_Get(a_Hashmap_t *const hashmap, const void *const key)
{
    void *value = NULL;

    if ((NULL != hashmap) && (NULL != key))
    {
        /* TODO */
    }

    return value;
}
