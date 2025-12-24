#ifndef AETHER_SESSION_H
#define AETHER_SESSION_H

#include <stdint.h>
#include <stddef.h>

#include "err.h"
#include "router.h"
#include "socket.h"
#include "tick.h"
#include "transport.h"

typedef enum
{
    A_SESSION_STATE_CONNECT,
    A_SESSION_STATE_ACCEPT,
    A_SESSION_STATE_OPEN,
    A_SESSION_STATE_CLOSED,
    A_SESSION_STATE_FAILED
} a_Session_State_t;

typedef struct
{
    a_Router_SessionId_t id;
    a_Session_State_t state;
    a_Tick_Ms_t lease;
    a_Tick_Ms_t last_renew_received;
    a_Tick_Ms_t last_renew_sent;
    uint8_t *buffer;
    size_t buffer_size;
} Session_t;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

void a_Session_SetPeerId(const a_Transport_PeerId_t *const id);
/* TODO use link mode to determine initial state */
a_Err_t a_Session_Initialize(Session_t *const session, a_Socket_t *const socket, uint8_t *const buffer, const size_t buffer_size);
/* TODO can probably be removed since sessions will delete themselves from router */
a_Err_t a_Session_GetState(const Session_t *const session, a_Session_State_t *const state);
a_Err_t a_Session_Task(Session_t *const session);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AETHER_SESSION_H */
