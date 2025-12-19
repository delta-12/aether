#ifndef AETHER_ROUTER_H
#define AETHER_ROUTER_H

#include "err.h"
#include "socket.h"
#include "transport.h"

typedef uint32_t a_Router_SessionId_t;

#define A_TRANSPORT_SESSION_ID_MAX (a_Router_SessionId_t)(UINT64_MAX)

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

a_Err_t a_Router_Initialize(void);
a_Err_t a_Router_SessionAdd(a_Router_SessionId_t *const id, a_Socket_t *const socket);
a_Err_t a_Router_SessionDelete(const a_Router_SessionId_t id);
a_Err_t a_Router_SessionMessageGet(const a_Router_SessionId_t id, a_Transport_Message_t *const message);
a_Err_t a_Router_SessionMessageSend(const a_Router_SessionId_t id, a_Transport_Message_t *const message);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AETHER_ROUTER_H */
