#ifndef AETHER_SERIAL_H
#define AETHER_SERIAL_H

#include <stddef.h>
#include <stdint.h>

#include "buffer.h"
#include "err.h"
#include "socket.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

a_Err_t a_Serial_Send(a_Socket_t *const socket, a_Buffer_t *const data);
a_Err_t a_Serial_Receive(a_Socket_t *const socket, a_Buffer_t *const data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AETHER_SERIAL_H */
