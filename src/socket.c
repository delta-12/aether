#include "socket.h"

#include <stddef.h>
#include <stdint.h>

#include "buffer.h"
#include "err.h"
#include "serial.h"

a_Err_t a_Socket_Initialize(a_Socket_t *const socket,
                            const a_Socket_Type_t type,
                            size_t (*send)(const uint8_t *const, const size_t),
                            uint8_t *const send_buffer,
                            const size_t send_buffer_size,
                            size_t (*receive)(uint8_t *const, const size_t),
                            uint8_t *const receive_buffer,
                            const size_t receive_buffer_size)
{
    a_Err_t error = A_ERR_NULL;

    /* TODO size check buffers */
    if ((NULL != socket) && (NULL != send) && (NULL != receive))
    {
        socket->type    = type;
        socket->send    = send;
        socket->receive = receive;

        error = a_Buffer_Initialize(&socket->send_buffer, send_buffer, send_buffer_size);

        if (A_ERR_NONE == error)
        {
            error = a_Buffer_Initialize(&socket->receive_buffer, receive_buffer, receive_buffer_size);
        }
    }

    return error;
}

a_Err_t a_Socket_Send(a_Socket_t *const socket, a_Buffer_t *const data)
{
    a_Err_t error = A_ERR_NULL;

    if ((NULL != socket) && (NULL != data))
    {
        switch (socket->type)
        {
        case A_SOCKET_TYPE_TCP:
            /* TODO */
            break;
        case A_SOCKET_TYPE_SERIAL:
            error = a_Serial_Send(socket, data);
            break;
        default:
            error = A_ERR_SOCKET;
            break;
        }
    }

    return error;
}

a_Err_t a_Socket_Receive(a_Socket_t *const socket, a_Buffer_t *const data)
{
    a_Err_t error = A_ERR_NULL;

    if ((NULL != socket) && (NULL != data))
    {
        switch (socket->type)
        {
        case A_SOCKET_TYPE_TCP:
            /* TODO */
            break;
        case A_SOCKET_TYPE_SERIAL:
            error = a_Serial_Receive(socket, data);
            break;
        default:
            error = A_ERR_SOCKET;
            break;
        }
    }

    return error;
}
