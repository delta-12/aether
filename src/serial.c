#include "serial.h"

#include <stddef.h>
#include <stdint.h>

#include "buffer.h"
#include "cobs.h"
#include "err.h"
#include "socket.h"

a_Err_t a_Serial_Send(a_Socket_t *const socket, a_Buffer_t *const data)
{
    a_Err_t error = A_ERR_NONE;

    if ((NULL == socket) || (NULL == data))
    {
        error = A_ERR_NULL;
    }
    else if (a_Buffer_GetReadSize(data) > 0U)
    {
        /* TODO CRC before COBS encode */
        size_t encoded = Cobs_Encode(a_Buffer_GetRead(data), a_Buffer_GetReadSize(data), a_Buffer_GetWrite(&socket->send_buffer), a_Buffer_GetWriteSize(&socket->send_buffer));

        if (SIZE_MAX == encoded)
        {
            error = A_ERR_SOCKET;
        }
        else
        {
            (void)a_Buffer_SetRead(data, a_Buffer_GetReadSize(data));
            (void)a_Buffer_SetWrite(&socket->send_buffer, encoded);

            size_t sent = socket->send(a_Buffer_GetRead(&socket->send_buffer), a_Buffer_GetReadSize(&socket->send_buffer));

            if (SIZE_MAX == sent)
            {
                error = A_ERR_SOCKET;
            }
            else
            {
                (void)a_Buffer_SetRead(&socket->send_buffer, sent);
            }
        }
    }

    return error;
}

a_Err_t a_Serial_Receive(a_Socket_t *const socket, a_Buffer_t *const data)
{
    a_Err_t error = A_ERR_NONE;

    if ((NULL == socket) || (NULL == data))
    {
        error = A_ERR_NULL;
    }
    else
    {
        uint8_t byte = UINT8_MAX;

        while ((0U != byte) && (a_Buffer_GetWriteSize(&socket->receive_buffer) > 0U))
        {
            size_t received = socket->receive(&byte, 1U);

            if (1U == received)
            {
                *a_Buffer_GetWrite(&socket->receive_buffer) = byte;
                (void)a_Buffer_SetWrite(&socket->receive_buffer, 1U);
            }
            else
            {
                break;
            }
        }

        if (0U == byte)
        {
            size_t decoded = Cobs_Decode(a_Buffer_GetWrite(data), a_Buffer_GetWriteSize(data), a_Buffer_GetRead(&socket->receive_buffer),
                                         a_Buffer_GetReadSize(&socket->receive_buffer));

            if (SIZE_MAX == decoded)
            {
                error = A_ERR_SOCKET;
            }
            else
            {
                (void)a_Buffer_SetWrite(data, decoded);
                (void)a_Buffer_SetRead(&socket->receive_buffer, a_Buffer_GetReadSize(&socket->receive_buffer));

                /* TODO check CRC after COBS decode */
            }
        }
        else if (a_Buffer_GetWriteSize(&socket->receive_buffer) > 0U)
        {
            error = A_ERR_SOCKET;
        }
    }

    return error;
}
