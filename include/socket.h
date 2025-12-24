#ifndef AETHER_SOCKET_H
#define AETHER_SOCKET_H

#include <stddef.h>
#include <stdint.h>

#include "buffer.h"
#include "err.h"

typedef enum
{
    A_SOCKET_TYPE_TCP,
    A_SOCKET_TYPE_SERIAL
} a_Socket_Type_t;

typedef struct
{
    a_Socket_Type_t type;
    size_t (*send)(const uint8_t *const data, const size_t size);
    size_t (*receive)(uint8_t *const data, const size_t size);
    a_Buffer_t send_buffer;
    a_Buffer_t receive_buffer;
} a_Socket_t;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

a_Err_t a_Socket_Initialize(a_Socket_t *const socket,
                            const a_Socket_Type_t type,
                            size_t (*send)(const uint8_t *const, const size_t),
                            uint8_t *const send_buffer,
                            const size_t send_buffer_size,
                            size_t (*receive)(uint8_t *const, const size_t),
                            uint8_t *const receive_buffer,
                            const size_t receive_buffer_size);
a_Err_t a_Socket_Send(a_Socket_t *const socket, a_Buffer_t *const data);
a_Err_t a_Socket_Receive(a_Socket_t *const socket, a_Buffer_t *const data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AETHER_SOCKET_H */
