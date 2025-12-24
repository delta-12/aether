#include "serial.h"

#include <stddef.h>
#include <stdint.h>

#include "buffer.h"
#include "cobs.h"
#include "err.h"

a_Err_t a_Serial_Send(size_t (*send)(const uint8_t *const data, const size_t size), a_Buffer_t *const data, a_Buffer_t *const buffer)
{
    a_Err_t error = A_ERR_NONE;

    if ((NULL == send) || (NULL == data) || (NULL == buffer))
    {
        error = A_ERR_NULL;
    }
    else if (a_Buffer_GetReadSize(data) > 0U)
    {
        /* TODO CRC before COBS encode */
        size_t encoded = Cobs_Encode(a_Buffer_GetRead(data), a_Buffer_GetReadSize(data), a_Buffer_GetWrite(buffer), a_Buffer_GetWriteSize(buffer));

        if (SIZE_MAX == encoded)
        {
            error = A_ERR_SOCKET;
        }
        else
        {
            (void)a_Buffer_SetRead(data, a_Buffer_GetReadSize(data));
            (void)a_Buffer_SetWrite(buffer, encoded);

            size_t sent = send(a_Buffer_GetRead(buffer), a_Buffer_GetReadSize(buffer));

            if (SIZE_MAX == sent)
            {
                error = A_ERR_SOCKET;
            }
            else
            {
                (void)a_Buffer_SetRead(buffer, sent);
            }
        }
    }

    return error;
}

a_Err_t a_Serial_Receive(size_t (*receive)(uint8_t *const data, const size_t size), a_Buffer_t *const data, a_Buffer_t *const buffer)
{
    a_Err_t error = A_ERR_NONE;

    if ((NULL == receive) || (NULL == data) || (NULL == buffer))
    {
        error = A_ERR_NULL;
    }
    else
    {
        uint8_t byte = UINT8_MAX;

        while ((0U != byte) && (a_Buffer_GetWriteSize(buffer) > 0U))
        {
            size_t received = receive(&byte, 1U);

            if (1U == received)
            {
                *a_Buffer_GetWrite(buffer) = byte;
                (void)a_Buffer_SetWrite(buffer, 1U);
            }
            else
            {
                break;
            }
        }

        if (0U == byte)
        {
            size_t decoded = Cobs_Decode(a_Buffer_GetWrite(data), a_Buffer_GetWriteSize(data), a_Buffer_GetRead(buffer), a_Buffer_GetReadSize(buffer));

            if (SIZE_MAX == decoded)
            {
                error = A_ERR_SOCKET;
            }
            else
            {
                (void)a_Buffer_SetWrite(data, decoded);
                (void)a_Buffer_SetRead(buffer, a_Buffer_GetReadSize(buffer));

                /* TODO check CRC after COBS decode */
            }
        }
        else if (a_Buffer_GetWriteSize(buffer) > 0U)
        {
            error = A_ERR_SOCKET;
        }
    }

    return error;
}
