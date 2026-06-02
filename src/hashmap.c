#include "hashmap.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "err.h"
#include "hash.h"
#include "memory.h"

#define A_HASHMAP_RESIZE_THRESHOLD_LOW  0.25
#define A_HASHMAP_RESIZE_THRESHOLD_HIGH 0.75
#define A_HASHMAP_RESIZE_FACTOR         2U

struct Hashmap_Entry
{
    void *key;
    size_t key_size;
    void *value;
    size_t value_size;
    a_Hashmap_Entry_t *next;
};

static a_Err_t a_Hashmap_Resize(a_Hashmap_t *const hashmap, const size_t capacity);
static double a_Hashmap_GetSizeFactor(const a_Hashmap_t *const hashmap);
static size_t a_Hashmap_GetIndex(const a_Hashmap_t *const hashmap, const void *const key, const size_t key_size);
static a_Hashmap_Entry_t *a_Hashmap_GetEntry(const a_Hashmap_t *const hashmap, const void *const key, const size_t key_size);
static void a_Hashmap_FreeEntry(a_Hashmap_Entry_t *const entry);

a_Err_t a_Hashmap_Initialize(a_Hashmap_t *const hashmap)
{
    a_Err_t error = A_ERR_NONE;

    if (NULL == hashmap)
    {
        error = A_ERR_NULL;
    }
    else
    {
        hashmap->capacity  = 1U;
        hashmap->size      = 0U;
        hashmap->iterating = 0U;
        hashmap->entries   = a_calloc(hashmap->capacity, sizeof(a_Hashmap_Entry_t *));

        if (NULL == hashmap->entries)
        {
            error = A_ERR_MEMORY;
        }
    }

    return error;
}

void a_Hashmap_Deinitialize(a_Hashmap_t *const hashmap)
{
    if (NULL != hashmap)
    {
        for (size_t i = 0U; i < hashmap->capacity; i++)
        {
            a_Hashmap_Entry_t *entry = *(hashmap->entries + i);

            while (NULL != entry)
            {
                a_Hashmap_Entry_t *next = entry->next;

                a_Hashmap_FreeEntry(entry);

                entry = next;
            }
        }

        a_free(hashmap->entries);
        hashmap->capacity  = 0U;
        hashmap->size      = 0U;
        hashmap->iterating = 0U;
    }
}

a_Err_t a_Hashmap_Insert(a_Hashmap_t *const hashmap, const void *const key, const size_t key_size, const void *const value, const size_t value_size)
{
    a_Err_t error = A_ERR_NONE;

    if ((NULL == hashmap) || (NULL == key) || (NULL == value))
    {
        error = A_ERR_NULL;
    }
    else if ((0U == key_size) || (0U == value_size))
    {
        error = A_ERR_SIZE;
    }
    else
    {
        a_Hashmap_Entry_t *entry = a_Hashmap_GetEntry(hashmap, key, key_size);

        if (NULL != entry)
        {
            if (value_size != entry->value_size)
            {
                entry->value = a_realloc(entry->value, value_size);
            }

            if (NULL == entry->value)
            {
                error = A_ERR_MEMORY;
            }
            else
            {
                memcpy(entry->value, value, value_size);
                entry->value_size = value_size;
            }
        }
        else
        {
            entry = a_malloc(sizeof(a_Hashmap_Entry_t));
            void *const entry_key   = a_malloc(key_size);
            void *const entry_value = a_malloc(value_size);

            if ((NULL == entry) || (NULL == entry_key) || (NULL == entry_value))
            {
                error = A_ERR_MEMORY;
            }
            else
            {
                const size_t index = a_Hashmap_GetIndex(hashmap, key, key_size);

                entry->key        = entry_key;
                entry->key_size   = key_size;
                entry->value      = entry_value;
                entry->value_size = value_size;
                entry->next       = *(hashmap->entries + index);

                memcpy(entry->key, key, entry->key_size);
                memcpy(entry->value, value, entry->value_size);

                *(hashmap->entries + index) = entry;
                hashmap->size++;
            }

            if ((a_Hashmap_GetSizeFactor(hashmap) > A_HASHMAP_RESIZE_THRESHOLD_HIGH) && (0U == hashmap->iterating))
            {
                error = a_Hashmap_Resize(hashmap, hashmap->capacity * A_HASHMAP_RESIZE_FACTOR);
            }
        }
    }

    return error;
}

void *a_Hashmap_Get(const a_Hashmap_t *const hashmap, const void *const key, const size_t key_size)
{
    void *value = NULL;

    if ((NULL != hashmap) && (NULL != key) && (0U != key_size))
    {
        const a_Hashmap_Entry_t *const entry = a_Hashmap_GetEntry(hashmap, key, key_size);

        if (NULL != entry)
        {
            value = entry->value;
        }
    }

    return value;
}

a_Err_t a_Hashmap_Remove(a_Hashmap_t *const hashmap, const void *const key, const size_t key_size)
{
    a_Err_t error = A_ERR_NONE;

    if ((NULL == hashmap) || (NULL == key))
    {
        error = A_ERR_NULL;
    }
    else if (0U == key_size)
    {
        error = A_ERR_SIZE;
    }
    else
    {
        a_Hashmap_Entry_t **previous = hashmap->entries + a_Hashmap_GetIndex(hashmap, key, key_size);
        a_Hashmap_Entry_t * entry    = *previous;

        while (NULL != entry)
        {
            if (0 == memcmp(entry->key, key, entry->key_size))
            {
                *previous = entry->next;
                a_Hashmap_FreeEntry(entry);
                hashmap->size--;

                if ((a_Hashmap_GetSizeFactor(hashmap) < A_HASHMAP_RESIZE_THRESHOLD_LOW) &&
                    (hashmap->size > 0U) &&
                    (0U == hashmap->iterating))
                {
                    error = a_Hashmap_Resize(hashmap, hashmap->capacity / A_HASHMAP_RESIZE_FACTOR);
                }

                break;
            }

            previous = &entry->next;
            entry    = entry->next;
        }
    }

    return error;
}

a_Err_t a_Hashmap_ForEach(a_Hashmap_t *const hashmap,
                          void (*callback)(const void *const key,
                                           const size_t key_size,
                                           void *const value,
                                           const size_t value_size,
                                           const void *const arg),
                          const void *const arg)
{
    a_Err_t error = A_ERR_NONE;

    if ((NULL == hashmap) || (NULL == callback))
    {
        error = A_ERR_NULL;
    }
    else
    {
        const size_t size = hashmap->size;

        hashmap->iterating++;

        for (size_t i = 0U; i < hashmap->capacity; i++)
        {
            a_Hashmap_Entry_t *entry = *(hashmap->entries + i);

            while (NULL != entry)
            {
                a_Hashmap_Entry_t *next = entry->next;

                callback(entry->key, entry->key_size, entry->value, entry->value_size, arg);

                entry = next;
            }
        }

        hashmap->iterating--;

        if ((size != hashmap->size) && (0U == hashmap->iterating))
        {
            const double size_factor = a_Hashmap_GetSizeFactor(hashmap);

            if (size_factor > A_HASHMAP_RESIZE_THRESHOLD_HIGH)
            {
                error = a_Hashmap_Resize(hashmap, hashmap->capacity * A_HASHMAP_RESIZE_FACTOR);
            }
            else if ((size_factor < A_HASHMAP_RESIZE_THRESHOLD_LOW) && (hashmap->size > 0U))
            {
                error = a_Hashmap_Resize(hashmap, hashmap->capacity / A_HASHMAP_RESIZE_FACTOR);
            }
        }
    }

    return error;
}

static a_Err_t a_Hashmap_Resize(a_Hashmap_t *const hashmap, const size_t capacity)
{
    a_Err_t             error       = A_ERR_MEMORY;
    a_Hashmap_Entry_t **new_entries = a_calloc(capacity, sizeof(a_Hashmap_Entry_t *));

    if (NULL != new_entries)
    {
        for (size_t i = 0; i < hashmap->capacity; i++)
        {
            a_Hashmap_Entry_t *entry = *(hashmap->entries + i);

            while (NULL != entry)
            {
                a_Hashmap_Entry_t **new_entry = new_entries + (a_Hash_Value(entry->key, entry->key_size) % capacity);
                a_Hashmap_Entry_t * next      = entry->next;

                entry->next = *new_entry;
                *new_entry  = entry;
                entry       = next;
            }
        }

        a_free(hashmap->entries);

        hashmap->entries  = new_entries;
        hashmap->capacity = capacity;

        error = A_ERR_NONE;
    }

    return error;
}

static double a_Hashmap_GetSizeFactor(const a_Hashmap_t *const hashmap)
{
    return (double)hashmap->size / (double)hashmap->capacity;
}

static size_t a_Hashmap_GetIndex(const a_Hashmap_t *const hashmap, const void *const key, const size_t key_size)
{
    return a_Hash_Value(key, key_size) % hashmap->capacity;
}

static a_Hashmap_Entry_t *a_Hashmap_GetEntry(const a_Hashmap_t *const hashmap, const void *const key, const size_t key_size)
{
    a_Hashmap_Entry_t *entry = *(hashmap->entries + a_Hashmap_GetIndex(hashmap, key, key_size));

    while (NULL != entry)
    {
        if (0 == memcmp(entry->key, key, key_size))
        {
            break;
        }

        entry = entry->next;
    }

    return entry;
}

static void a_Hashmap_FreeEntry(a_Hashmap_Entry_t *const entry)
{
    a_free(entry->key);
    a_free(entry->value);
    a_free(entry);
}
