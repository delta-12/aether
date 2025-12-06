#include "buffer.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "err.h"

a_Err_t a_Buffer_Initialize(a_Buffer_t *const buffer, uint8_t *const data, const size_t size)
{
    a_Err_t error = A_ERR_NULL;

    if ((NULL != buffer) && (NULL != data))
    {
        buffer->data  = data;
        buffer->size  = size;
        buffer->write = 0U;
        buffer->read  = 0U;

        error = A_ERR_NONE;
    }

    return error;
}

a_Err_t a_Buffer_Clear(a_Buffer_t *const buffer)
{
    a_Err_t error = A_ERR_NULL;

    if (NULL != buffer)
    {
        buffer->write = 0U;
        buffer->read  = 0U;

        error = A_ERR_NONE;
    }

    return error;
}

a_Err_t a_Buffer_SetWrite(a_Buffer_t *const buffer, const size_t written)
{
    a_Err_t error = A_ERR_NULL;

    if (NULL != buffer)
    {
        if (written <= a_Buffer_GetWriteSize(buffer))
        {
            buffer->write += written;
            error          = A_ERR_NONE;
        }
        else
        {
            error = A_ERR_SIZE;
        }
    }

    return error;
}

a_Err_t a_Buffer_SetRead(a_Buffer_t *const buffer, const size_t read)
{
    a_Err_t error = A_ERR_NULL;

    if (NULL != buffer)
    {
        if (read <= a_Buffer_GetReadSize(buffer))
        {
            buffer->read += read;
            error         = A_ERR_NONE;
        }
        else
        {
            error = A_ERR_SIZE;
        }
    }

    return error;
}

uint8_t *a_Buffer_GetWrite(const a_Buffer_t *const buffer)
{
    uint8_t *write = NULL;

    if ((NULL != buffer) && (buffer->write < buffer->size))
    {
        write = buffer->data + buffer->write;
    }

    return write;
}

uint8_t *a_Buffer_GetRead(const a_Buffer_t *const buffer)
{
    uint8_t *read = NULL;

    if ((NULL != buffer) && (buffer->read < buffer->write))
    {
        read = buffer->data + buffer->read;
    }

    return read;
}

size_t a_Buffer_GetWriteSize(const a_Buffer_t *const buffer)
{
    size_t size = 0U;

    if (NULL != buffer)
    {
        size = buffer->size - buffer->write;
    }

    return size;
}

size_t a_Buffer_GetReadSize(const a_Buffer_t *const buffer)
{
    size_t size = 0U;

    if (NULL != buffer)
    {
        size = buffer->write - buffer->read;
    }

    return size;
}

a_Err_t a_Buffer_AppendLeft(a_Buffer_t *const buffer, const a_Buffer_t *const appended)
{
    a_Err_t error = A_ERR_NONE;

    if ((NULL == buffer) || (NULL == appended))
    {
        error = A_ERR_NULL;
    }
    else if (a_Buffer_GetWriteSize(buffer) < a_Buffer_GetReadSize(appended))
    {
        error = A_ERR_SIZE;
    }
    else
    {
        size_t   size        = a_Buffer_GetReadSize(appended);
        uint8_t *buffer_read = buffer->data + buffer->read;
        memmove((buffer_read + size), buffer_read, a_Buffer_GetReadSize(buffer));
        memcpy(buffer_read, (appended->data + appended->read), size);
        (void)a_Buffer_SetWrite(buffer, size);
    }

    return error;
}

a_Err_t a_Buffer_AppendRight(a_Buffer_t *const buffer, const a_Buffer_t *const appended)
{
    a_Err_t error = A_ERR_NONE;

    if ((NULL == buffer) || (NULL == appended))
    {
        error = A_ERR_NULL;
    }
    else if (a_Buffer_GetWriteSize(buffer) < a_Buffer_GetReadSize(appended))
    {
        error = A_ERR_SIZE;
    }
    else
    {
        size_t size = a_Buffer_GetReadSize(appended);
        memcpy((buffer->data + buffer->write), (appended->data + appended->read), size);
        (void)a_Buffer_SetWrite(buffer, size);
    }

    return error;
}
