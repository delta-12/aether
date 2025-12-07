#include "router.h"

#include "err.h"
#include "socket.h"
#include "transport.h"

/* TODO internally track PID+SEQ and return error to drop message if necessary*/

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

    /* TODO */

    return A_ERR_NONE;
}

a_Err_t a_Router_SessionMessageSend(const a_Router_SessionId_t id, a_Transport_Message_t *const message)
{
    A_UNUSED(id);
    A_UNUSED(message);

    /* TODO */

    return A_ERR_NONE;
}
