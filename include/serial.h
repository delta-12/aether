#ifndef AETHER_SERIAL_H
#define AETHER_SERIAL_H

#include <stddef.h>
#include <stdint.h>

#include "buffer.h"
#include "err.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

a_Err_t a_Serial_Send(size_t (*send)(const uint8_t *const data, const size_t size), a_Buffer_t *const data, a_Buffer_t *const buffer);
a_Err_t a_Serial_Receive(size_t (*receive)(uint8_t *const data, const size_t size), a_Buffer_t *const data, a_Buffer_t *const buffer);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AETHER_SERIAL_H */
