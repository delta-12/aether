#include "buffer.h"

#include <stddef.h>
#include <stdint.h>

#include "types.h"

a_Error_t a_Buffer_Initialize(a_Buffer_t *const buffer, uint8_t *const data, const size_t size)
{
    a_Error_t error = A_ERROR_NULL;

    if ((NULL != buffer) && (NULL != data))
    {
        buffer->data     = data;
        buffer->size     = size;
        buffer->position = 0U;

        error = A_ERROR_NONE;
    }

    return error;
}

a_Error_t a_Buffer_Clear(a_Buffer_t *const buffer)
{
    a_Error_t error = A_ERROR_NULL;

    if (NULL != buffer)
    {
        buffer->position = 0U;

        error = A_ERROR_NONE;
    }

    return error;
}

a_Error_t a_Buffer_SetWrite(a_Buffer_t *const buffer, const size_t written)
{
    a_Error_t error = A_ERROR_NULL;

    if (NULL != buffer)
    {
        size_t max_write = buffer->size - buffer->position;

        if (written <= max_write)
        {
            buffer->position += written;
            error             = A_ERROR_NONE;
        }
        else
        {
            error = A_ERROR_SIZE;
        }
    }

    return error;
}

a_Error_t a_Buffer_SetRead(a_Buffer_t *const buffer, const size_t read)
{
    a_Error_t error = A_ERROR_NULL;

    if (NULL != buffer)
    {
        if (read <= buffer->position)
        {
            buffer->position -= read;
            error             = A_ERROR_NONE;
        }
        else
        {
            error = A_ERROR_SIZE;
        }
    }

    return error;
}
