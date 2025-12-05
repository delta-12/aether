#ifndef AETHER_BUFFER_H
#define AETHER_BUFFER_H

#include <stddef.h>
#include <stdint.h>

#include "error.h"

typedef struct
{
    uint8_t *data;
    size_t size;
    size_t position;
} a_Buffer_t;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

a_Error_t a_Buffer_Initialize(a_Buffer_t *const buffer, uint8_t *const data, const size_t size);
a_Error_t a_Buffer_Clear(a_Buffer_t *const buffer);
a_Error_t a_Buffer_SetWrite(a_Buffer_t *const buffer, const size_t written);
a_Error_t a_Buffer_SetRead(a_Buffer_t *const buffer, const size_t read);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AETHER_BUFFER_H */
