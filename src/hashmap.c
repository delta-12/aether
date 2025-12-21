#include "hashmap.h"

#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "err.h"

#define A_HASHMAP_SENTINEL  0xFFU
#define A_HASHMAP_HASH_SEED 5381U /* DJB2 seed */

static void a_Hashmap_SetRowColumnSize(a_Hashmap_t *const hashmap, uint8_t *const data, const size_t size);
static unsigned long a_Hashmap_Hash(const void *const key, const size_t size);
static uint8_t *a_Hashmap_GetRow(const a_Hashmap_t *const hashmap, const void *const key);
static uint8_t *a_Hashmap_GetColumn(uint8_t *const row, const size_t column, const size_t entry_size);
static bool a_Hashmap_HasEntry(const uint8_t *const entry, const size_t key_size);
static bool a_Hashmap_HasKey(const uint8_t *const entry, const void *const key, const size_t key_size);

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
        hashmap->key_size   = key_size;
        hashmap->entry_size = hashmap->key_size + value_size;
        a_Hashmap_SetRowColumnSize(hashmap, data, size);
    }

    return error;
}

a_Err_t a_Hashmap_Insert(const a_Hashmap_t *const hashmap, const void *const key, const void *const value)
{
    a_Err_t error = A_ERR_NULL;

    if ((NULL != hashmap) && (NULL != key) && (NULL != value))
    {
        uint8_t *const row          = a_Hashmap_GetRow(hashmap, key);
        const size_t   value_size   = hashmap->entry_size - hashmap->key_size;
        size_t         empty_column = hashmap->columns;
        size_t         column       = hashmap->columns;
        do
        {
            column--;

            uint8_t *const entry = a_Hashmap_GetColumn(row, column, hashmap->entry_size);

            if (a_Hashmap_HasKey(entry, key, hashmap->key_size))
            {
                memcpy((entry + hashmap->key_size), value, value_size);
                error = A_ERR_NONE;
                break;
            }
            else if (!a_Hashmap_HasEntry(entry, hashmap->key_size))
            {
                empty_column = column;
            }
        }
        while (0U != column);

        if ((A_ERR_NONE != error) && (empty_column < hashmap->columns))
        {
            memcpy((a_Hashmap_GetColumn(row, empty_column, hashmap->entry_size) + hashmap->key_size), value, value_size);
            error = A_ERR_NONE;
        }
        else
        {
            error = A_ERR_SIZE;
        }
    }

    return error;
}

void *a_Hashmap_Get(const a_Hashmap_t *const hashmap, const void *const key)
{
    void *value = NULL;

    if ((NULL != hashmap) && (NULL != key))
    {
        void *row = a_Hashmap_GetRow(hashmap, key);

        for (size_t column = 0U; column < hashmap->columns; column++)
        {
            uint8_t *const entry = a_Hashmap_GetColumn(row, column, hashmap->entry_size);

            if (0 == memcmp(entry, key, hashmap->key_size))
            {
                value = (void *)(entry + hashmap->key_size);
                break;
            }
        }
    }

    return value;
}

a_Err_t a_Hashmap_Remove(const a_Hashmap_t *const hashmap, const void *const key)
{
    a_Err_t error = A_ERR_NONE;

    if ((NULL == hashmap) || (NULL == key))
    {
        error = A_ERR_NULL;
    }
    else
    {
        uint8_t *const value = a_Hashmap_Get(hashmap, key);

        if (NULL != value)
        {
            memset((value - hashmap->key_size), A_HASHMAP_SENTINEL, hashmap->key_size);
        }
    }

    return error;
}

static void a_Hashmap_SetRowColumnSize(a_Hashmap_t *const hashmap, uint8_t *const data, const size_t size)
{
    const size_t max_entries    = size / hashmap->entry_size;
    const size_t max_columns    = (size_t)sqrt((double)max_entries) + 1U;
    size_t       rows           = max_entries;
    size_t       columns        = 1U;
    size_t       min_difference = rows - columns;
    for (size_t i = 2U; i < max_columns; i++)
    {
        if (max_entries % i == 0U)
        {
            size_t new_rows   = max_entries / i;
            size_t difference = new_rows - i;

            if (difference < min_difference)
            {
                min_difference = difference;
                rows           = new_rows;
                columns        = i;
            }
        }
    }

    hashmap->data    = data;
    hashmap->rows    = rows;
    hashmap->columns = columns;
    memset(data, A_HASHMAP_SENTINEL, size);
}

static unsigned long a_Hashmap_Hash(const void *const key, const size_t size)
{
    /* DJB2 Hash Function */

    unsigned long hash = A_HASHMAP_HASH_SEED;

    for (size_t i = 0U; i < size; i++)
    {
        hash = ((hash << 5U) + hash) + *((const uint8_t *const)key + i);
    }

    return hash;
}

static uint8_t *a_Hashmap_GetRow(const a_Hashmap_t *const hashmap, const void *const key)
{
    return hashmap->data + ((a_Hashmap_Hash(key, hashmap->key_size) % hashmap->rows) * hashmap->columns * hashmap->entry_size);
}

static uint8_t *a_Hashmap_GetColumn(uint8_t *const row, const size_t column, const size_t entry_size)
{
    return row + (entry_size * column);
}

static bool a_Hashmap_HasEntry(const uint8_t *const entry, const size_t key_size)
{
    bool has_entry = false;

    for (size_t i = 0U; i < key_size; i++)
    {
        if (A_HASHMAP_SENTINEL != *(entry + i))
        {
            has_entry = true;
            break;
        }
    }

    return has_entry;
}

static bool a_Hashmap_HasKey(const uint8_t *const entry, const void *const key, const size_t key_size)
{
    return 0 == memcmp(entry, key, key_size);
}
