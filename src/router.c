#include "router.h"

#include "err.h"
#include "hashmap.h"
#include "socket.h"
#include "transport.h"

/* TODO internally track PID+SEQ and return error to drop message if necessary*/

#ifndef AETHER_ROUTER_MAX_SESSIONS
#define AETHER_ROUTER_MAX_SESSIONS 16U
#endif /* AETHER_ROUTER_MAX_SESSIONS */

static a_Hashmap_t sockets;
static uint8_t     sockets_data[(sizeof(a_Router_SessionId_t) + sizeof(a_Socket_t)) * AETHER_ROUTER_MAX_SESSIONS];

a_Err_t a_Router_Initialize(void)
{
    return a_Hashmap_Initialize(&sockets, sockets_data, sizeof(sockets_data), sizeof(a_Router_SessionId_t), sizeof(a_Socket_t));
}

a_Err_t a_Router_SessionAdd(a_Router_SessionId_t *const id, a_Socket_t *const socket)
{
    A_UNUSED(id);
    A_UNUSED(socket);

    /* TODO */

    return A_ERR_NONE;
}

a_Err_t a_Router_SessionDelete(const a_Router_SessionId_t id)
{
    A_UNUSED(id);

    /* TODO */

    return A_ERR_NONE;
}

a_Err_t a_Router_SessionMessageGet(const a_Router_SessionId_t id, a_Transport_Message_t *const message)
{
    A_UNUSED(id);
    A_UNUSED(message);

    a_Buffer_t *buffer = a_Transport_GetMessageBuffer(message);
    a_Err_t     error  = a_Socket_Receive(a_Hashmap_Get(&sockets, &id), buffer);

    if ((A_ERR_NONE == error) && (a_Buffer_GetReadSize(buffer) > 0U))
    {
        error = a_Transport_DeserializeMessage(message);

        /* TODO update routing info */
    }

    if (A_ERR_NONE != error)
    {
        /* TODO log error */
    }

    return error;

    return A_ERR_NONE;
}

a_Err_t a_Router_SessionMessageSend(const a_Router_SessionId_t id, a_Transport_Message_t *const message)
{
    A_UNUSED(id);
    A_UNUSED(message);

    (void)a_Transport_SerializeMessage(message);
    a_Err_t error = a_Socket_Send(a_Hashmap_Get(&sockets, &id), a_Transport_GetMessageBuffer(message));

    if (A_ERR_NONE != error)
    {
        /* TODO log error */
    }

    return error;

    return A_ERR_NONE;
}
