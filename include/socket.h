#ifndef AETHER_SOCKET_H
#define AETHER_SOCKET_H

#include <stddef.h>
#include <stdint.h>

#include "buffer.h"
#include "err.h"

typedef enum
{
    FLOW_STREAM,
    FLOW_DATAGRAM
} a_Socket_Flow_t;

typedef struct
{
    a_Socket_Flow_t flow;
    size_t (*send)(const uint8_t *const data, const size_t size);
    size_t (*receive)(uint8_t *const data, const size_t size);
} a_Socket_t;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

a_Err_t a_Socket_Send(a_Socket_t *const socket, a_Buffer_t *const buffer);
a_Err_t a_Socket_Receive(a_Socket_t *const socket, a_Buffer_t *const buffer);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AETHER_SOCKET_H */
