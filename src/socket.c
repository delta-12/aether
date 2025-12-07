#include "socket.h"

#include "buffer.h"
#include "err.h"

a_Err_t a_Socket_Send(a_Socket_t *const socket, a_Buffer_t *const buffer)
{
    A_UNUSED(socket);
    A_UNUSED(buffer);

    /* TODO */
    /* TODO NULL check function pointers */

    return A_ERR_SOCKET;
}

a_Err_t a_Socket_Receive(a_Socket_t *const socket, a_Buffer_t *const buffer)
{
    A_UNUSED(socket);
    A_UNUSED(buffer);

    /* TODO */

    return A_ERR_SOCKET;
}
