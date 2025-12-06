#ifndef AETHER_SESSION_H
#define AETHER_SESSION_H

#include <stdint.h>
#include <stddef.h>

#include "err.h"
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
    a_Transport_SessionId_t id;
    a_Socket_t socket;
    a_Session_State_t state;
    a_Tick_Ms_t lease;
    uint8_t *send_buffer;
    size_t send_buffer_size;
    uint8_t *receive_buffer;
    size_t receive_buffer_size;
} Session_t;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

void a_Session_SetPeerId(const a_Transport_PeerId_t *const id);
/* TODO use link mode to determine initial state */
a_Err_t a_Session_Initialize(Session_t *const session,
                             const a_Transport_SessionId_t id,
                             a_Socket_t *const socket,
                             uint8_t *const send_buffer,
                             const size_t send_buffer_size,
                             uint8_t *const receive_buffer,
                             const size_t receive_buffer_size);
a_Err_t a_Session_GetState(Session_t *const session, a_Session_State_t *const state);
a_Err_t a_Session_Task(Session_t *const session);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AETHER_SESSION_H */
