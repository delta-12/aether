#ifndef AETHER_ROUTER_H
#define AETHER_ROUTER_H

#include <stdbool.h>

#include "err.h"
#include "socket.h"
#include "transport.h"

typedef uint32_t a_Router_SessionId_t;

#define A_TRANSPORT_SESSION_ID_MAX (a_Router_SessionId_t)(UINT64_MAX)

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

a_Err_t a_Router_Initialize(const a_Transport_PeerId_t id);
void a_Router_Deinitialize(void);
void a_Routing_EnableRouting(const bool enable);
void a_Router_Task(void);
a_Err_t a_Router_SessionAdd(const a_Router_SessionId_t id, const a_Socket_t *const socket, uint8_t *const buffer, const size_t size, const bool retain);
a_Err_t a_Router_SessionDelete(const a_Router_SessionId_t id);
a_Err_t a_Router_Declare(const char *const key);
a_Err_t a_Router_Publish(const char *const key, const uint8_t *const data, const size_t size);
a_Err_t a_Router_Subscribe(const char *const key, void (*callback)(const char *const key, const uint8_t *const data, const size_t size, void *arg), void *arg);
a_Err_t a_Router_Unsubscribe(const char *const key);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AETHER_ROUTER_H */
